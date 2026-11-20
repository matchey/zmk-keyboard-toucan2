if(CONFIG_ZMK_INPUT_PROCESSOR_GESTURE_KEY)
  if ((NOT CONFIG_ZMK_SPLIT) OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    target_sources(app PRIVATE
      src/input_processor_gesture_key.c
    )
  endif()
endif()
