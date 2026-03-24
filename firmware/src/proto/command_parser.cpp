#include "proto/command_parser.h"

#include <Arduino.h>
#include <string.h>
#include <stdlib.h>

#include "app/app.h"

namespace
{
    constexpr uint8_t LINE_BUFFER_SIZE = 24;
    char g_line[LINE_BUFFER_SIZE];
    uint8_t g_line_len = 0;

    static void write_error(int code)
    {
        Serial.print(F("ERR CODE="));
        Serial.println(code);
    }

    static void handle_stream_command(const char *raw)
    {
        if (strncmp(raw, "O ", 2) == 0)
        {
            const int value = atoi(raw + 2);

            if (!set_output(value))
            {
                write_error(3);
            }

            return;
        }

        if (strcmp(raw, "S") == 0)
        {
            handle_motor(true);
            return;
        }

        if (strcmp(raw, "x") == 0)
        {
            handle_motor(false);
            return;
        }

        write_error(1);
    }
}

void process_command_parser()
{
    while (Serial.available() > 0)
    {
        char c = static_cast<char>(Serial.read());

        if (c == '\r')
        {
            continue;
        }

        if (c == '\n')
        {
            if (g_line_len > 0)
            {
                handle_stream_command(g_line);

                g_line_len = 0;
                g_line[0] = '\0';
            }
        }
        else
        {
            if (g_line_len < LINE_BUFFER_SIZE - 1)
            {
                g_line[g_line_len++] = c;
                g_line[g_line_len] = '\0';
            }
            else
            {
                g_line_len = 0;
                g_line[0] = '\0';

                write_error(2);
            }
        }
    }
}
