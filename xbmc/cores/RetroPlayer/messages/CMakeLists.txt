set(MESSAGES action.fbs
             action_controller.fbs
             action_keyboard.fbs
             action_topology.fbs
             multicodec.fbs
             savestate.fbs
)

core_add_messages(retroplayer_messages)
set(EXTRA_INCLUDES ${EXTRA_INCLUDES} PARENT_SCOPE)
