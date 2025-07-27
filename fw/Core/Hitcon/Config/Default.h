#ifndef HITCON_CONFIG_DEFAULT_H_
#define HITCON_CONFIG_DEFAULT_H_
// Default configuration for the Hitcon badge firmware.
// Override defaults in custom.h if needed.

#define BADGE_ROLE_ATTENDEE 1
#define BADGE_ROLE_SPONSOR 2

#ifndef BADGE_ROLE
#define BADGE_ROLE BADGE_ROLE_ATTENDEE
#endif  // BADGE_ROLE

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
