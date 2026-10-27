/**
 * \file    database.hpp
 * \brief   Declares the DBC-backed CAN message database.
 *
 * Defines Can::Model::Database, a lightweight, non-owning view over
 * null-terminated DBC text. The class searches DBC \c BO_ message rows and
 * extracts the identifier and payload length needed to construct a
 * Can::Model::Message or Can::Model::CyclicMessage with caller-provided
 * signal data pointers.
 *
 * The parser currently supports exact message-name lookup and validates the
 * parsed identifier, declared payload length, and combined signal data size.
 * It does not parse signal definitions, node assignments, DBC attributes, or
 * cycle times. Messages can only be built through the consteval builders;
 * missing definitions and mismatched payload sizes fail compilation.
 *
 * The DBC text is not copied and must remain valid and null-terminated while
 * the Database is used. Signal data referenced by a returned message must
 * likewise outlive that message's use. Successfully built messages are
 * returned by value and require no manual memory management.
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

namespace Can::Model {

/**
 * \brief   Builds CAN messages from DBC message definitions.
 *
 * This class reads a null-terminated DBC string without taking ownership of
 * it. The current parser recognizes message header rows beginning with
 * \c BO_ and extracts only the decimal CAN identifier, exact message name,
 * and declared payload length. Signal definitions, node assignments,
 * attributes, and cycle times are not parsed.
 *
 * Message data is checked against the declared payload length and the
 * classic-CAN maximum of eight bytes. The data type sizes are taken from C++
 * \c sizeof; they are not compared with individual DBC signal definitions.
 *
 * The DBC text is a non-owning pointer and must remain valid and
 * null-terminated while the database is used. Cycle times are supplied by the
 * caller when creating cyclic messages; they are not read from DBC metadata.
 */
class Database
{
public:
    /**
     * \brief   Creates a database view over DBC text.
     *
     * Stores non-owning pointers; it does not copy either string or parse the
     * DBC during construction.
     *
     * \param   file passes the DBC content as a null-terminated string.
     */
    constexpr Database(const char* file) :
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
        const char* name,
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
     * must match a supported DBC message. The cycle time is supplied by the
     * caller and is expressed in milliseconds.
     *
     * \tparam SignalDataTypes specifies the types of the signal data.
     * \param name passes the exact DBC message name to find.
     * \param cycleTime passes the cycle time in milliseconds.
     * \param signalPtrPack passes non-owning pointers to the signal data.
     *
     * \return The matching CyclicMessage.
     */
    template<typename... SignalDataTypes>
        requires(AllowedSignalDataType<SignalDataTypes> && ...) &&
                    FitsIntoCanMessage<SignalDataTypes...>
    consteval CyclicMessage buildCyclicMessage(
        const char* name,
        const uint16_t cycleTime,
        SignalDataTypes*... signalPtrPack
    ) const {
        const std::optional<MessageDefinition> definition =
            findMessageWithPayloadSize(
                name, (0 + ... + sizeof(SignalDataTypes))
            );

        if (!definition)
            failConstantEvaluation();

        return CyclicMessage{
            definition->identifier, cycleTime, signalPtrPack...
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
     * The identifier and payload length are read from a single DBC \c BO_
     * header row. This does not represent or retain the row's signal
     * definitions or other DBC metadata.
     */
    struct MessageDefinition {
        uint16_t identifier;
        uint8_t payloadSize;
    };

    /**
     * \brief   Searches the DBC text for a message with an exact name.
     *
     * Scans the input one newline-delimited row at a time and delegates
     * parsing of each potential \c BO_ row. A name match is case-sensitive. A 
     * null name or DBC pointer, or the absence of a supported matching row,
     * produces an empty optional.
     *
     * \param name passes the exact message name to locate.
     * 
     * \return The parsed identifier and payload length, or an empty optional.
     */
    constexpr std::optional<MessageDefinition> findMessage(
        const char* name
    ) const {
        if (!name || !_file)
            return std::nullopt;

        for (const char* line = _file; *line; ) {
            const char* lineEnd = line;
            while (*lineEnd && '\n' != *lineEnd)
                ++lineEnd;
            
            const std::optional<MessageDefinition> definition = 
                parseMessageDefinition(line, lineEnd, name);
            
            if (definition)
                return definition;

            line = *lineEnd ? lineEnd + 1 : lineEnd;
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
        const char* name,
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
     * \brief   Parses the identifier, name, and payload length of one row.
     *
     * The row must start exactly with \c BO_ followed by an unsigned decimal
     * identifier, the requested exact name, a colon, and an unsigned decimal
     * payload length. The identifier must fit in 16 bits and the payload
     * length in 8 bits. Other row forms and malformed values are rejected.
     *
     * \param line passes the first character of the row.
     * \param lineEnd passes the one-past-the-end pointer for the row.
     * \param name passes an exact name the row must contain.
     * 
     * \return Parsed fields for a valid matching row, otherwise empty.
     */
    static constexpr std::optional<MessageDefinition> parseMessageDefinition(
        const char* line, 
        const char* lineEnd, 
        const char* name
    ) {
        if (lineEnd - line < 4 ||
            'B' != line[0] ||
            'O' != line[1] ||
            '_' != line[2] ||
            ' ' != line[3]
        )
            return std::nullopt;

        const char* cursor = line + 4;
        uint32_t identifier = 0;

        if (!parseUnsigned(cursor, lineEnd, identifier))
            return std::nullopt;
        
        if (!parseMessageName(cursor, lineEnd, name))
            return std::nullopt;
        
        uint32_t payloadSize = 0;
        
        if (!parseUnsigned(cursor, lineEnd, payloadSize))
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
     * On success, cursor points to the first character after the colon.
     *
     * \param cursor passes current position in the row; advanced on success.
     * \param end passes one-past-the-end pointer for the row.
     * \param name passes an exact name to compare with the row.
     * 
     * \return True if the name matches and the colon is present.
     */
    static constexpr bool parseMessageName(
        const char*& cursor, 
        const char* end, 
        const char* name
    ) {
        while (cursor < end && (' ' == *cursor || '\t' == *cursor))
            ++cursor;

        const char* i = cursor;

        while (i < end && ':' != *i && ' ' != *i && '\t' != *i)
            ++i;

        std::size_t nameLength = 0;
        while (name[nameLength])
            ++nameLength;

        const std::size_t length = static_cast<std::size_t>(i - cursor);
        bool isMatch = length == nameLength;
        for (std::size_t index = 0; isMatch && index < length; ++index)
            isMatch = cursor[index] == name[index];

        if (i == end || ':' != *i || !isMatch)
            return false;

        cursor = i + 1;
        return true;
    }

    /**
     * \brief   Parses an unsigned decimal integer from a bounded row segment.
     *
     * Skips leading spaces and tabs, then consumes consecutive decimal
     * digits. Parsing fails if there are no digits or the value would
     * overflow uint32_t. On success, cursor points to the first character
     * after the digits; trailing characters are left for the caller to
     * interpret.
     *
     * \param cursor passes the current position.
     * \param end passes one-past-the-end pointer restricting the read.
     * \param value outputs the parsed integer on success.
     * 
     * \return True if at least one digit was parsed without overflow.
     */
    static constexpr bool parseUnsigned(
        const char*& cursor, 
        const char* end, 
        uint32_t& value
    ) {
        while (cursor < end && (' ' == *cursor || '\t' == *cursor))
            ++cursor;

        if (cursor == end || *cursor < '0' || *cursor > '9')
            return false;
        
        value = 0;
        
        while (cursor < end && *cursor >= '0' && *cursor <= '9') {
            const uint32_t digit = static_cast<uint32_t>(*cursor++ - '0');
            
            if (value > (std::numeric_limits<uint32_t>::max() - digit) / 10)
                return false;

            value = value * 10 + digit;
        }

        return true;
    }

    /**
     * \brief   Maximum payload size supported by the classic-CAN Message model.
     */
    static constexpr uint8_t _maximumPayloadSize = 8;

    /**
     * \brief   Non-owning pointer to the complete null-terminated DBC text.
     *
     * Must remain valid for every lookup performed by this Database.
     */
    const char* _file;
};

}

#endif //__MERCURYCAN_MODEL_DATABASE_HPP__
