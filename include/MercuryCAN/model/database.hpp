/**
 * \file    database.hpp
 * \brief   Declares the DBC-backed CAN message database.
 *
 * Defines Can::Model::Database, a lightweight, non-owning view over DBC text.
 * The class searches DBC \c BO_ message rows and extracts the identifier and
 * payload length needed to construct a
 * Can::Model::Message or Can::Model::CyclicMessage with caller-provided
 * signal data pointers.
 *
 * The parser supports exact message-name lookup and validates the parsed
 * identifier, declared payload length, and combined signal data size. Cyclic
 * messages additionally require a cyclic \c GenMsgSendType and a defined
 * \c GenMsgCycleTime. Messages can only be built through the consteval
 * builders; missing definitions and mismatched payload sizes fail compilation.
 *
 * The DBC text is not copied and must remain valid while the Database is
 * used. Signal data referenced by a returned message must likewise outlive
 * that message's use. Successfully built messages are returned by value and
 * require no manual memory management.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MERCURYCAN_MODEL_DATABASE_HPP__
#define __MERCURYCAN_MODEL_DATABASE_HPP__

#include <MercuryCAN/model/cyclicMessage.hpp>
#include <MercuryCAN/model/message.hpp>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string_view>

namespace Can::Model {

/**
 * \brief   Builds CAN messages from DBC message definitions.
 *
 * This class reads DBC text through a non-owning string view. The current
 * parser recognizes message header rows beginning with \c BO_ and extracts
 * the decimal CAN identifier, exact message name, and declared payload
 * length. For cyclic messages it also reads \c GenMsgSendType and
 * \c GenMsgCycleTime attributes. Signal definitions and node assignments are
 * not parsed.
 *
 * Message data is checked against the declared payload length and the
 * classic-CAN maximum of eight bytes. The data type sizes are taken from C++
 * \c sizeof; they are not compared with individual DBC signal definitions.
 *
 * The DBC text is a non-owning view and must remain valid while the database
 * is used. Cyclic message timing and send type are read from DBC attributes.
 */
class Database
{
public:
    /**
     * \brief   Creates a database view over DBC text.
     *
     * Stores a non-owning view; it does not copy or parse the DBC during
     * construction.
     *
     * \param   file passes the DBC content.
     */
    constexpr Database(std::string_view file) :
        _file(file)
    {}

    /**
     * \brief   Builds a message matching a DBC message definition.
     *
     * Searches for an exact, case-sensitive message name in a \c BO_ row.
     * A successful lookup supplies the CAN identifier and establishes the 
     * maximum data size from the DBC payload length. The combined \c sizeof of 
     * the signal data types must fit both that length and the eight-byte 
     * classic-CAN limit. Individual DBC signal names, bit offsets, and signal 
     * lengths are not checked.
     *
     * Each signal pointer must be non-null and point to data that remains
     * alive for at least as long as the returned Message is used. Pointers
     * are passed through to Message and are not owned or copied.
     *
     * \tparam SignalDataTypes specifies types of the signal data.
     * \param name passes the exact DBC message name to find.
     * \param signalPtrPack passes non-owning pointers to the signal data.
     * 
     * \return The matching Message.
     */
    template<typename... SignalDataTypes>
        requires(AllowedSignalDataType<SignalDataTypes> && ...) &&
                    FitsIntoCanMessage<SignalDataTypes...>
    consteval Message buildMessage(
        std::string_view name,
        SignalDataTypes*... signalPtrPack
    ) const {
        const std::optional<MessageDefinition> definition =
            findMessageWithPayloadSize(
                name, (0 + ... + sizeof(SignalDataTypes))
            );

        if (!definition)
            failConstantEvaluation();

        return Message{definition->identifier, signalPtrPack...};
    }

    /**
     * \brief   Builds a cyclic message matching a DBC message definition.
     *
     * The exact, case-sensitive message name and combined signal data size
     * must match a supported DBC message. The message must have a cyclic
     * \c GenMsgSendType (\c cyclic, \c cyclicIfActive, \c cyclicAndTriggered,
     * or \c cyclicIfActiveAndTriggered) and a defined \c GenMsgCycleTime,
     * expressed in milliseconds.
     *
     * \tparam SignalDataTypes specifies the types of the signal data.
     * \param name passes the exact DBC message name to find.
     * \param signalPtrPack passes non-owning pointers to the signal data.
     *
     * \return The matching CyclicMessage.
     */
    template<typename... SignalDataTypes>
        requires(AllowedSignalDataType<SignalDataTypes> && ...) &&
                    FitsIntoCanMessage<SignalDataTypes...>
    consteval CyclicMessage buildCyclicMessage(
        std::string_view name,
        SignalDataTypes*... signalPtrPack
    ) const {
        const std::optional<MessageDefinition> definition =
            findCyclicMessageWithPayloadSize(
                name, (0 + ... + sizeof(SignalDataTypes))
            );

        if (!definition)
            failConstantEvaluation();

        return CyclicMessage{
            definition->identifier, definition->cycleTime, signalPtrPack...
        };
    }

private:
    /**
     * \brief   Causes invalid consteval builds to fail during constant evaluation.
     */
    static void failConstantEvaluation() {}

    /**
     * \brief   Parsed fields needed to construct a Message.
     *
     * The identifier and payload length are read from a DBC \c BO_ header
     * row. Cyclic-message lookup additionally fills cycleTime from its DBC
     * attributes.
     */
    struct MessageDefinition {
        uint16_t identifier;
        uint8_t payloadSize;
        uint16_t cycleTime = 0;
    };

    /**
     * \brief   Searches the DBC text for a message with an exact name.
     *
     * Scans the input one newline-delimited row at a time and delegates
     * parsing of each potential \c BO_ row. A name match is case-sensitive.
     * An empty name or the absence of a supported matching row produces an
     * empty optional.
     *
     * \param name passes the exact message name to locate.
     * 
     * \return The parsed identifier and payload length, or an empty optional.
     */
    constexpr std::optional<MessageDefinition> findMessage(
        std::string_view name
    ) const {
        if (name.empty() || _file.empty())
            return std::nullopt;

        std::size_t lineStart = 0;
        while (lineStart < _file.size()) {
            const std::size_t newline = _file.find('\n', lineStart);
            const std::size_t lineEnd =
                std::string_view::npos == newline ? _file.size() : newline;
            const std::optional<MessageDefinition> definition =
                parseMessageDefinition(
                    _file.substr(lineStart, lineEnd - lineStart), name
                );

            if (definition)
                return definition;

            if (std::string_view::npos == newline)
                break;

            lineStart = newline + 1;
        }

        return std::nullopt;
    }

    /**
     * \brief   Finds a message whose declared payload matches the signal data.
     *
     * \param name passes the exact DBC message name to find.
     * \param signalSize passes the combined size of the signal data types.
     *
     * \return The parsed definition when its payload is supported and matches
     *         signalSize, otherwise an empty optional.
     */
    constexpr std::optional<MessageDefinition> findMessageWithPayloadSize(
        std::string_view name,
        const std::size_t signalSize
    ) const {
        const std::optional<MessageDefinition> definition = findMessage(name);

        if (!definition ||
            definition->payloadSize > _maximumPayloadSize ||
            signalSize != definition->payloadSize
        )
            return std::nullopt;

        return definition;
    }

    /**
     * \brief   Finds a cyclic message with matching signal data and attributes.
     */
    constexpr std::optional<MessageDefinition>
    findCyclicMessageWithPayloadSize(
        std::string_view name,
        const std::size_t signalSize
    ) const {
        std::optional<MessageDefinition> definition =
            findMessageWithPayloadSize(name, signalSize);

        if (!definition)
            return std::nullopt;

        bool hasCyclicSendType = false;
        std::optional<uint16_t> cycleTime;

        std::size_t lineStart = 0;
        while (lineStart < _file.size()) {
            const std::size_t newline = _file.find('\n', lineStart);
            const std::size_t lineEnd =
                std::string_view::npos == newline ? _file.size() : newline;
            const std::string_view line =
                _file.substr(lineStart, lineEnd - lineStart);
            uint32_t identifier = 0;
            std::string_view value;

            if (parseAttributeHeader(
                    line, "GenMsgSendType", identifier, value
                ) && identifier == definition->identifier) {
                hasCyclicSendType = parseCyclicSendType(value);
            } else if (parseAttributeHeader(
                           line, "GenMsgCycleTime",
                           identifier, value
                       ) && identifier == definition->identifier) {
                uint32_t parsedCycleTime = 0;
                if (parseUnsigned(value, parsedCycleTime) &&
                    hasAttributeTerminator(value) &&
                    parsedCycleTime <=
                        std::numeric_limits<uint16_t>::max()) {
                    cycleTime = static_cast<uint16_t>(parsedCycleTime);
                } else {
                    cycleTime.reset();
                }
            }

            if (std::string_view::npos == newline)
                break;

            lineStart = newline + 1;
        }

        if (!hasCyclicSendType || !cycleTime)
            return std::nullopt;

        definition->cycleTime = *cycleTime;
        return definition;
    }

    /**
     * \brief   Parses the header of a DBC message attribute assignment.
     *
     * On success, value views the attribute value and identifier holds the BO_ 
     * identifier.
     */
    static constexpr bool parseAttributeHeader(
        std::string_view line,
        std::string_view attributeName,
        uint32_t& identifier,
        std::string_view& value
    ) {
        skipHorizontalWhitespace(line);

        if (!line.starts_with("BA_"))
            return false;

        line.remove_prefix(3);
        skipHorizontalWhitespace(line);

        if (!parseQuotedString(line, attributeName))
            return false;

        skipHorizontalWhitespace(line);

        if (!line.starts_with("BO_"))
            return false;

        line.remove_prefix(3);
        if (!parseUnsigned(line, identifier))
            return false;

        skipHorizontalWhitespace(line);
        value = line;
        return true;
    }

    /**
     * \brief   Parses and compares a quoted DBC string value.
     */
    static constexpr bool parseQuotedString(
        std::string_view& cursor,
        std::string_view expected
    ) {
        if (cursor.empty() || '"' != cursor.front())
            return false;

        cursor.remove_prefix(1);

        if (cursor.size() <= expected.size() ||
            cursor.substr(0, expected.size()) != expected ||
            '"' != cursor[expected.size()])
            return false;

        cursor.remove_prefix(expected.size() + 1);
        return true;
    }

    /**
     * \brief   Checks whether a send type includes cyclic transmission.
     */
    static constexpr bool parseCyclicSendType(std::string_view value) {
        const auto matches = [value](std::string_view sendType) {
            auto cursor = value;
            return parseQuotedString(cursor, sendType) &&
                   hasAttributeTerminator(cursor);
        };

        return matches("cyclic") ||
               matches("cyclicIfActive") ||
               matches("cyclicAndTriggered") ||
               matches("cyclicIfActiveAndTriggered");
    }

    /**
     * \brief   Checks for the assignment terminator after an attribute value.
     */
    static constexpr bool hasAttributeTerminator(
        std::string_view text
    ) {
        skipAttributeWhitespace(text);

        if (text.empty() || ';' != text.front())
            return false;

        text.remove_prefix(1);
        skipAttributeWhitespace(text);

        return text.empty();
    }

    /**
     * \brief   Parses the identifier, name, and payload length of one row.
     *
     * The row must start exactly with \c BO_ followed by an unsigned decimal
     * identifier, the requested exact name, a colon, and an unsigned decimal
     * payload length. The identifier must fit in 16 bits and the payload
     * length in 8 bits. Other row forms and malformed values are rejected.
     *
     * \param line passes the row text.
     * \param name passes an exact name the row must contain.
     * 
     * \return Parsed fields for a valid matching row, otherwise empty.
     */
    static constexpr std::optional<MessageDefinition> parseMessageDefinition(
        std::string_view line,
        std::string_view name
    ) {
        if (!line.starts_with("BO_ "))
            return std::nullopt;

        line.remove_prefix(4);
        uint32_t identifier = 0;

        if (!parseUnsigned(line, identifier))
            return std::nullopt;

        if (!parseMessageName(line, name))
            return std::nullopt;

        uint32_t payloadSize = 0;

        if (!parseUnsigned(line, payloadSize))
            return std::nullopt;

        if (identifier > std::numeric_limits<uint16_t>::max() ||
            payloadSize > std::numeric_limits<uint8_t>::max()
        )
            return std::nullopt;

        return MessageDefinition{
            static_cast<uint16_t>(identifier),
            static_cast<uint8_t>(payloadSize)
        };
    }

    /**
     * \brief   Matches a DBC name field and advances past its colon.
     *
     * Leading spaces and tabs before the name are ignored. The name must be
     * followed immediately by a colon; matching is exact and case-sensitive.
     * On success, cursor starts with the text after the colon.
     *
     * \param cursor passes the remaining row text; shortened on success.
     * \param name passes an exact name to compare with the row.
     * 
     * \return True if the name matches and the colon is present.
     */
    static constexpr bool parseMessageName(
        std::string_view& cursor,
        std::string_view name
    ) {
        skipHorizontalWhitespace(cursor);
        const std::size_t nameEnd = cursor.find_first_of(" \t:");

        if (std::string_view::npos == nameEnd ||
            ':' != cursor[nameEnd] ||
            cursor.substr(0, nameEnd) != name)
            return false;

        cursor.remove_prefix(nameEnd + 1);
        return true;
    }

    /**
     * \brief   Parses an unsigned decimal integer from a bounded row segment.
     *
     * Skips leading spaces and tabs, then consumes consecutive decimal
     * digits. Parsing fails if there are no digits or the value would
     * overflow uint32_t. On success, cursor starts at the first character
     * after the digits; trailing characters remain for the caller to interpret.
     *
     * \param cursor passes the remaining text; shortened on success.
     * \param value outputs the parsed integer on success.
     * 
     * \return True if at least one digit was parsed without overflow.
     */
    static constexpr bool parseUnsigned(
        std::string_view& cursor,
        uint32_t& value
    ) {
        skipHorizontalWhitespace(cursor);

        if (cursor.empty() || cursor.front() < '0' || cursor.front() > '9')
            return false;

        value = 0;

        while (!cursor.empty() &&
               cursor.front() >= '0' && cursor.front() <= '9') {
            const uint32_t digit =
                static_cast<uint32_t>(cursor.front() - '0');
            cursor.remove_prefix(1);

            if (value > (std::numeric_limits<uint32_t>::max() - digit) / 10)
                return false;

            value = value * 10 + digit;
        }

        return true;
    }

    static constexpr void skipHorizontalWhitespace(std::string_view& text) {
        while (!text.empty() && (' ' == text.front() || '\t' == text.front()))
            text.remove_prefix(1);
    }

    static constexpr void skipAttributeWhitespace(std::string_view& text) {
        while (!text.empty() &&
               (' ' == text.front() ||
                '\t' == text.front() ||
                '\r' == text.front()))
            text.remove_prefix(1);
    }

    /**
     * \brief   Maximum payload size supported by the classic-CAN Message model.
     */
    static constexpr uint8_t _maximumPayloadSize = 8;

    /**
     * \brief   Non-owning view of the complete DBC text.
     *
     * The referenced text must remain valid for every lookup performed by
     * this Database.
     */
    std::string_view _file;
};

}

#endif //__MERCURYCAN_MODEL_DATABASE_HPP__
