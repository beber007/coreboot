ifeq ($(CONFIG_SOC_AMD_COMMON_BLOCK_UART),y)

all-$(CONFIG_AMD_SOC_CONSOLE_UART) += uart_console.c

ifeq ($(CONFIG_DEBUG_SMI),y)
smm-$(CONFIG_AMD_SOC_CONSOLE_UART) += uart_console.c
endif

ramstage-$(CONFIG_HAVE_ACPI_TABLES) += uart_acpi.c

endif # CONFIG_SOC_AMD_COMMON_BLOCK_UART
