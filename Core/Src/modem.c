#include "modem.h"
#include "c-logger.h"
#include "interface/pconfig.h"
#include "bsp/adc_bsp.h"
#include "bsp/ptt_bsp.h"
#include "bsp/dac_bsp.h"

static int _init_decoder(modem_handle_t *handle);
static int _handle_tx(modem_handle_t *handle);
static int _handle_rx(modem_handle_t *handle);

int modem_init(modem_handle_t *handle, orchestrator_handle_t *orchestrator_ctx)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return -1;
    }

    memset(handle, 0, sizeof(modem_handle_t));

    handle->orchestrator_ctx = orchestrator_ctx; // Callback for sending packets to orchestrator when decoded

    if (_init_decoder(handle))
    {
        LOG_ERROR("Failed to init decoder");
        return -1;
    }

    time_utils_start(&handle->symbol_timer, (ONE_SECOND / pconfigBAUD_RATE)); // Start symbol timer based on baud rate
    time_utils_start(&handle->ptt_timer, pconfigPTT_DELAY_MS * ONE_MS);       // Start PTT delay timer

    if (circular_buffer_dynamic_init(&handle->tx_buffer, pconfigMODEM_TX_BUFFER_SIZE, sizeof(uint8_t)))
    {
        LOG_ERROR("Failed to init modem TX buffer");
        return -1;
    }

    handle->transmitting = false;

    // Setup BSP components
    if (adc_bsp_init())
    {
        LOG_ERROR("Failed to init ADC BSP");
        return -1;
    }
    if (ptt_bsp_init())
    {
        LOG_ERROR("Failed to init PTT BSP");
        return -1;
    }
    if (dac_bsp_init())
    {
        LOG_ERROR("Failed to init DAC BSP");
        return -1;
    }

    bit_unpacker_init(&handle->bit_unpacker);

failed:
    return ret;
}

int modem_send(modem_handle_t *handle, circular_buffer_t *cb)
{
    int ret = 0;

    if (!handle || !cb)
    {
        LOG_ERROR("Handle or circular buffer is NULL");
        return -1;
    }

    while (circular_buffer_count(cb) > 0)
    {
        uint8_t byte;
        if (circular_buffer_read(cb, &byte, sizeof(uint8_t)) != 0)
        {
            LOG_ERROR("Failed to read from cb");
            return -1;
        }

        if (circular_buffer_write(&handle->tx_buffer, &byte, sizeof(uint8_t)) != 0)
        {
            LOG_ERROR("Failed to write to modem tx_buffer");
            return -1;
        }
    }

    dac_bsp_set_tone(pconfigMODEM_FREQ_0); // Set this so recevers can detect line busy ASAP
    time_utils_reset(&handle->ptt_timer);  // Reset PTT timer to start delay before transmission
    ptt_bsp_set_ptt(true);                 // Set PTT high to start transmission
    handle->transmitting = true;

    return ret;
}

bool modem_rx_busy(modem_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return false;
    }

    return decoder_is_receiving(&handle->decoder);
}

bool modem_tx_busy(modem_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return false;
    }

    return handle->transmitting;
}

bool modem_busy(modem_handle_t *handle)
{
    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return false;
    }

    return modem_rx_busy(handle) || modem_tx_busy(handle);
}

int modem_task(modem_handle_t *handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return -1;
    }

    if (decoder_task(&handle->decoder))
    {
        LOG_ERROR("Decoder task failed");
        return -1;
    }
    if (adc_bsp_task())
    {
        LOG_ERROR("ADC BSP task failed");
        return -1;
    }
    if (ptt_bsp_task())
    {
        LOG_ERROR("PTT BSP task failed");
        return -1;
    }
    if (dac_bsp_task())
    {
        LOG_ERROR("DAC BSP task failed");
        return -1;
    }

    if (_handle_tx(handle))
    {
        LOG_ERROR("Failed to handle TX");
        return -1;
    }
    if (_handle_rx(handle))
    {
        LOG_ERROR("Failed to handle RX");
        return -1;
    }
}

// =-=-=-=-=-=-=-=-=-=
// PRIVATE FUNCTIONS
// =-=-=-=-=-=-=-=-=-=

static int _init_decoder(modem_handle_t *handle)
{
    int ret = 0;

    // Initialize decoder
    double sample_rate = calculate_sample_rate(pconfigMODEM_FREQ_0, pconfigMODEM_FREQ_1, pconfigBAUD_RATE);
    size_t samples_per_bit = (size_t)(sample_rate / pconfigBAUD_RATE);

    if (decoder_init(&handle->decoder))
    {
        LOG_ERROR("Failed to init decoder");
        return -1;
    }

    // FSK Decoder
    if (fsk_decoder_init(&handle->fsk_decoder))
    {
        LOG_ERROR("Failed to init FSK decoder");
        return -1;
    }
    if (fsk_decoder_set_symbol_sample_size(&handle->fsk_decoder, samples_per_bit, pconfigDECODER_BUFFER_SYMBOL_COUNT)) // Buffer for 3 symbols to allow for timing recovery
    {
        LOG_ERROR("Failed to set FSK decoder symbol sample size");
        return -1;
    }
    if (fsk_decoder_set_sample_rate(&handle->fsk_decoder, (int)sample_rate))
    {
        LOG_ERROR("Failed to set FSK decoder sample rate");
        return -1;
    }
    if (fsk_decoder_set_frequencies(&handle->fsk_decoder, pconfigMODEM_FREQ_0, pconfigMODEM_FREQ_1))
    {
        LOG_ERROR("Failed to set FSK decoder frequencies");
        return -1;
    }
    if (fsk_decoder_set_power_threshold(&handle->fsk_decoder, pconfigFSK_POWER_THRESHOLD))
    {
        LOG_ERROR("Failed to set FSK decoder power threshold");
        return -1;
    }
    if (decoder_set_bit_decoder(&handle->decoder, BIT_DECODER_FSK, &handle->fsk_decoder))
    {
        LOG_ERROR("Failed to set FSK bit decoder");
        return -1;
    }

    // Byte Assembler
    if (byte_assembler_init(&handle->byte_assembler))
    {
        LOG_ERROR("Failed to init byte assembler");
        return -1;
    }
    if (byte_assembler_set_preamble(&handle->byte_assembler, pconfigPREAMBLE_BYTE_1 << 8 | pconfigPREAMBLE_BYTE_2))
    {
        LOG_ERROR("Failed to set byte assembler preamble");
        return -1;
    }
    if (decoder_set_byte_decoder(&handle->decoder, BYTE_DECODER_BIT_STUFFING, &handle->byte_assembler))
    {
        LOG_ERROR("Failed to set byte decoder");
        return -1;
    }

    if (decoder_set_output_buffer_size(&handle->decoder, pconfigDECODER_OUTPUT_BUFFER_SIZE))
    {
        LOG_ERROR("Failed to set decoder output buffer size");
        return -1;
    }

failed:
    return ret;
}

int _handle_tx(modem_handle_t *handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return -1;
    }

    if (!time_utils_done(&handle->ptt_timer))
    {
        return 0; // Not time to send next symbol yet
    }

    if (!time_utils_done(&handle->symbol_timer))
    {
        return 0; // Not time to send next symbol yet
    }
    time_utils_reset(&handle->symbol_timer);

    if (bit_unpacker_empty(&handle->bit_unpacker) && !circular_buffer_count(&handle->tx_buffer)) // No more data to send
    {
        dac_bsp_set_tone(pconfigMODEM_FREQ_0); // Stop transmission
        ptt_bsp_set_ptt(false);                // Set PTT low to end transmission
        handle->transmitting = false;
        return 0;
    }

    bool bit;
    bit_unpacker_pop(&handle->bit_unpacker, &handle->tx_buffer, &bit);
    if (bit)
    {
        dac_bsp_set_tone(pconfigMODEM_FREQ_1);
    }
    else
    {
        dac_bsp_set_tone(pconfigMODEM_FREQ_0);
    }
}

int _handle_rx(modem_handle_t *handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return -1;
    }

    if (adc_bsp_data_available())
    {
        // Get samples from ADC and push to decoder input buffer
        if (adc_bsp_get_data(&handle->decoder.input_buffer))
        {
            LOG_ERROR("Failed to get ADC data");
            return -1;
        }
    }

    if (decoder_has_packet(&handle->decoder))
    {
        packet_t packet;
        if (decoder_get_packet(&handle->decoder, &packet))
        {
            LOG_ERROR("Failed to get decoded packet");
            return -1;
        }

        // Send packet to orchestrator for processing
        if (orchestrator_packet_callback(handle->orchestrator_ctx, &packet))
        {
            LOG_ERROR("Failed to send packet to orchestrator");
            return -1;
        }
    }

    return ret;
}