set(sources
    main.c
    uart.c
    wifi.c
)

idf_component_register(SRCS ${sources}
                    INCLUDE_DIRS ".")
