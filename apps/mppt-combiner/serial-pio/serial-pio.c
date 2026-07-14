#include "serial-pio.h"

serialpio_rx_buffer_t rxbuf = {0};


// inline bool buffer_check_if_empty(void) {
//     return rxbuf.head == rxbuf.tail;
// }
//
// inline bool buffer_check_if_full(void) {
//     return ((rxbuf.head + 1u) % SERIALPIO_BUF_SIZE) == rxbuf.tail;
// }
//
// inline void buffer_push(uint8_t c) {
//     uint16_t next = (rxbuf.head + 1u) % SERIALPIO_BUF_SIZE;
//     if (next == rxbuf.tail) {
//         rxbuf.dropped++;
//         return;
//     }
//     rxbuf.data[rxbuf.head] = c;
//     rxbuf.head = next;
// }
//
// inline int buffer_read(void) {
//     if (buffer_check_if_empty()) { return -1; }
//
//     uint8_t c = rxbuf.data[rxbuf.tail];
//     rxbuf.tail = (rxbuf.tail + 1u) % SERIALPIO_BUF_SIZE;
//     return c;
// }

// inline uint16_t serialpio_available(void) {
//     if (rxbuf.head >= rxbuf.tail) {
//         return rxbuf.head - rxbuf.tail;
//     }
//     return SERIALPIO_BUF_SIZE - rxbuf.tail + rxbuf.head;
// }


