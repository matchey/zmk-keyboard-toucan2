if(CONFIG_ZMK_INPUT_PROCESSOR_GESTURE_KEY)
  if ((NOT CONFIG_ZMK_SPLIT) OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    target_sources(app PRIVATE
      src/input_processor_gesture_key.c
    )
  endif()
endif()

if(CONFIG_ZMK_BEHAVIOR_LAYER_OFF)
  if ((NOT CONFIG_ZMK_SPLIT) OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    target_sources(app PRIVATE
      src/behavior_layer_off.c
    )
  endif()
endif()
