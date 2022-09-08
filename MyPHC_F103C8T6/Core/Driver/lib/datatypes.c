#include "datatypes.h"

#include "string.h"

uint8_t CheckVaild(uint8_t* buffer, uint32_t len) {
    if (buffer[0] + 3 != len) return 0;
    uint16_t crc_val;
    memcpy(&crc_val, buffer + len - 2, 2);
    uint16_t crc_chk = CRC16_Modbus_calc(buffer, len - 2);
    return crc_chk == crc_val;
}
void DataToBuffer(general_data* data, uint8_t* buffer) {
    buffer[0] = data->len;
    memcpy(buffer + 1, data->data, data->len);
    memcpy(buffer + 1 + data->len, &data->crc16, 2);
}

void BufferToData(uint8_t* buffer, general_data* data) {
    data->len = buffer[0];
    memcpy(data->data, buffer + 1, data->len);
    memcpy(&data->crc16, buffer + 1 + data->len, 2);
}

uint8_t BufferToData_Check(uint8_t* buffer, uint32_t len, general_data* data) {
    uint8_t valid = CheckVaild(buffer, len);
    if (!valid) return valid;
    BufferToData(buffer, data);
    return valid;
}