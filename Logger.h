#pragma once

#include <iostream>

#define LOGGER_USE_COLORS 1

#define LOGGER_COLOR_LOG "\033[94m"
#define LOGGER_COLOR_WARN "\033[93m"
#define LOGGER_COLOR_ERR "\033[91m"
#define LOGGER_COLOR_RESET "\033[0m"

namespace Logger
{

/*
 * Controls the logger
 */
enum Control
{
    End, // Can be used to mark the end of the line
};

class Logger final
{
public:
    /*
     * The type of the logger
     */
    enum class Type
    {
        Log,
        Warning,
        Error,
    };

private:
    // Whether this is the beginning of the line
    bool m_isBeginning{true};
    // The logger type: info, error, etc.
    Type m_type{};

public:
    Logger(Type type)
        : m_type{type}
    {
    }

    template <typename T>
    Logger& operator<<(const T &value)
    {
        // If this is the beginning of the line, print the "initial"
        // depending on the logger type
        if (m_isBeginning)
        {
            switch (m_type)
            {
            case Type::Log:
#ifdef LOGGER_USE_COLORS
                std::cout << LOGGER_COLOR_LOG << "[INFO]: " << LOGGER_COLOR_RESET;
#else
                std::cout << "[INFO]: ";
#endif
                break;

            case Type::Warning:
#ifdef LOGGER_USE_COLORS
                std::cout << LOGGER_COLOR_WARN << "[WARN]: " << LOGGER_COLOR_RESET;
#else
                std::cout << "[WARN]: ";
#endif
                break;

            case Type::Error:
#ifdef LOGGER_USE_COLORS
                std::cout << LOGGER_COLOR_ERR << "[ERR]: " << LOGGER_COLOR_RESET;
#else
                std::cout << "[ERR]: ";
#endif
                break;
            }

            // This is not the beginning of the line anymore
            m_isBeginning = false;
        }
        std::cout << value;

        // Make the operator chainable
        return *this;
    }

    Logger& operator<<(Control ctrl)
    {
        if (ctrl == End)
        {
            std::cout << "\n";
            // We printed the \n, to this is the beginning of the new line
            m_isBeginning = true;
        }

        // Make the operator chainable
        return *this;
    }
};

/*
 * Logger object instances with different types
 */
extern Logger log;
extern Logger warn;
extern Logger err;

} // End of namespace Logger

