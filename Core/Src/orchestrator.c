#include "orchestrator.h"
#include "c-logger.h"
#include "interface/pconfig.h"

int orchestrator_init(orchestrator_handle_t *handle)
{
    int ret = 0;

    if (!handle)
    {
        LOG_ERROR("Handle is NULL");
        return -1;
    }


}
int orchestrator_deinit(orchestrator_handle_t *handle);

int orchestrator_send(orchestrator_handle_t *handle, const uint8_t *data, size_t len);
int orchestrator_data_available(orchestrator_handle_t *handle);
int orchestrator_read(orchestrator_handle_t *handle, uint8_t *data, size_t len);
int orchestrator_process_incoming(orchestrator_handle_t *handle, const uint16_t *samples, size_t num_samples);

int orchestrator_task(orchestrator_handle_t *handle);