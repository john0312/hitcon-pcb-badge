#ifndef HITCON_CONFIG_DEFAULT_H_
#define HITCON_CONFIG_DEFAULT_H_
// Default configuration for the Hitcon badge firmware.
// Override defaults in custom.h if needed.

#define BADGE_ROLE_ATTENDEE 1
#define BADGE_ROLE_QR_STN 2

#ifndef BADGE_ROLE
#define BADGE_ROLE BADGE_ROLE_ATTENDEE
#endif  // BADGE_ROLE

// For BADGE_ROLE_QR_STN: which station id this is (0/1/2).
// The client uses the value to unlock one of the three QR code thirds.
#ifndef STATION_ID
#define STATION_ID 0
#endif  // STATION_ID

#if BADGE_ROLE == BADGE_ROLE_QR_STN
#if (STATION_ID < 0) || (STATION_ID > 2)
#error "STATION_ID must be in [0, 2] when BADGE_ROLE == BADGE_ROLE_QR_STN"
#endif
#endif  // BADGE_ROLE == BADGE_ROLE_QR_STN

/*
Example usage:

#include <Hitcon.h>

#ifndef BADGE_ROLE
#error "BADGE_ROLE not defined"
#endif  // BADGE_ROLE

#if BADGE_ROLE == BADGE_ROLE_ATTENDEE
...
#endif
*/

#endif /* HITCON_CONFIG_DEFAULT_H_ */
