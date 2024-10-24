set(sources
    main.c
    wifi.c
)

idf_component_register(SRCS ${sources}
                    INCLUDE_DIRS ".")
