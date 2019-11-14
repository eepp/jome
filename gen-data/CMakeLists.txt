# Copyright (C) 2019 Philippe Proulx <eepp.ca>
#
# This software may be modified and distributed under the terms
# of the MIT license. See the LICENSE file for details.

set (
    JOME-DATA-FILES
    "${JOME-DATA-DIR}/emojis.json"
    "${JOME-DATA-DIR}/cats.json"
    "${JOME-DATA-DIR}/emojis-png-locations-16.json"
    "${JOME-DATA-DIR}/emojis-16.png"
    "${JOME-DATA-DIR}/emojis-png-locations-24.json"
    "${JOME-DATA-DIR}/emojis-24.png"
    "${JOME-DATA-DIR}/emojis-png-locations-32.json"
    "${JOME-DATA-DIR}/emojis-32.png"
    "${JOME-DATA-DIR}/emojis-png-locations-40.json"
    "${JOME-DATA-DIR}/emojis-40.png"
    "${JOME-DATA-DIR}/emojis-png-locations-48.json"
    "${JOME-DATA-DIR}/emojis-48.png"
)
add_custom_command (
    OUTPUT ${JOME-DATA-FILES}
    COMMAND python3 create.py "${JOME-DATA-DIR}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    VERBATIM
)
add_custom_target (
    data
    DEPENDS ${JOME-DATA-FILES}
    VERBATIM
)
install (
    FILES ${JOME-DATA-FILES}
    DESTINATION share/jome/data
)
