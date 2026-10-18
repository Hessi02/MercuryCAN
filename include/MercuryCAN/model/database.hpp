/**
 * \file    database.hpp
 * \brief   Declares the DBC-backed CAN message database.
 *
 * Defines Can::Model::Database, a lightweight, non-owning view over
 * null-terminated DBC text. The class searches DBC \c BO_ message rows and
 * extracts the identifier and payload length needed to construct a
 * Can::Model::Message with caller-provided signal data pointers.
 *
 * The parser currently supports exact message-name lookup and validates the
 * parsed identifier, declared payload length, and combined signal data size.
 * It does not parse signal definitions, node assignments, DBC attributes, or
 * cycle times. Message creation returns an empty optional when no supported
 * matching definition is found or when the signal data does not fit.
 *
 * The DBC text and node name are not copied. The DBC text must remain valid
 * and null-terminated while the Database is used; signal data referenced by
 * a returned Message must likewise outlive that Message's use.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MERCURYCAN_MODEL_DATABASE_HPP__
#define __MERCURYCAN_MODEL_DATABASE_HPP__

#include <MercuryCAN/model/message.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>
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
 * Both constructor arguments are non-owning pointers. The DBC text must
 * remain valid and null-terminated while the database is used. The node name
 * is retained but is not currently used to filter or validate message rows.
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
     * \param   node passes the associated node name.
     * \param   file passes the DBC content as a null-terminated string.
     */
    constexpr Database(const char* node, const char* file) :
        _node(node),
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
     * \return A populated optional when row is found and the signal size fits.
     */
    template<typename... SignalDataTypes>
        requires(AllowedSignalDataType<SignalDataTypes> && ...) &&
                    FitsIntoCanMessage<SignalDataTypes...>
    std::optional<Message> buildMessage(
        const char* name, 
        SignalDataTypes*... signalPtrPack
    ) const {
        const std::optional<MessageDefinition> definition = findMessage(name);
        const std::size_t signalSize = (0 + ... + sizeof(SignalDataTypes));

        if (!definition)
            return std::nullopt;

        const bool payloadFits = definition->payloadSize <= _maximumPayloadSize;
        const bool signalDataFits = signalSize == definition->payloadSize;

        if (!payloadFits || !signalDataFits) 
            return std::nullopt;

        return std::optional<Message>(
            std::in_place, 
            definition->identifier, 
            signalPtrPack...
        );
    }

private:
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
    std::optional<MessageDefinition> findMessage(const char* name) const {
        if (!name || !_file)
            return std::nullopt;

        for (const char* line = _file; *line; ) {
            const char* lineEnd = std::strchr(line, '\n');

            if (!lineEnd)
                lineEnd = line + std::strlen(line);
            
            const std::optional<MessageDefinition> definition = 
                parseMessageDefinition(line, lineEnd, name);
            
            if (definition)
                return definition;

            line = *lineEnd ? lineEnd + 1 : lineEnd;
        }

        return std::nullopt;
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
    static std::optional<MessageDefinition> parseMessageDefinition(
        const char* line, 
        const char* lineEnd, 
        const char* name
    ) {
        if (lineEnd - line < 4 || 0 != std::strncmp(line, "BO_ ", 4))
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
    static bool parseMessageName(
        const char*& cursor, 
        const char* end, 
        const char* name
    ) {
        while (cursor < end && (' ' == *cursor || '\t' == *cursor))
            ++cursor;

        const char* i = cursor;

        while (i < end && ':' != *i && ' ' != *i && '\t' != *i)
            ++i;

        const std::size_t length = static_cast<std::size_t>(i - cursor);
        const bool isSameSize =  length == std::strlen(name);
        const bool isMatch = 0 == std::strncmp(cursor, name, i - cursor);

        if (i == end || ':' != *i || !isSameSize || !isMatch)
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
    static bool parseUnsigned(
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
     * \brief   Non-owning pointer to the associated node name.
     *
     * Retained for future node-aware database parsing; currently unused.
     */
    const char* _node;

    /**
     * \brief   Non-owning pointer to the complete null-terminated DBC text.
     *
     * Must remain valid for every lookup performed by this Database.
     */
    const char* _file;
};

}

#endif //__MERCURYCAN_MODEL_DATABASE_HPP__
