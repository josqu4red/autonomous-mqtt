set(sources
    desk.c
    mqtt.c
    uart.c
    wifi.c
    main.c
)

idf_component_register(SRCS ${sources}
                    INCLUDE_DIRS ".")
