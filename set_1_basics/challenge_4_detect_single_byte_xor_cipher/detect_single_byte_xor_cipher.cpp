#include "detect_single_byte_xor_cipher.hpp"
#include <map>
#include "line_extract_streambuf.hpp"
#include "decode_hex.hpp"
#include <sstream>
#include <utility>
#include <stdexcept>

namespace cryptopals {

std::tuple<unsigned int, unsigned int, uint8_t> detect_single_byte_xor_cipher(
        std::ostream & output,
        std::istream & input,
        break_single_byte_xor_cipher & mechanism
)
{
    std::map<unsigned int,
             std::tuple<unsigned int, std::istream::pos_type, uint8_t>>
             scoreLinePosAndKeyMap;

    for (unsigned int lineNumber = 0; input; ++lineNumber) {
        try {
            std::istream::pos_type startPos = input.tellg();
            line_extract_streambuf lineExtractor(input);
            std::istream lineExtractStream(&lineExtractor);
            decode_hex_streambuf hexDecoder(lineExtractStream);
            std::ostringstream plaintextStream;
            std::pair<unsigned int, uint8_t> scoreAndXorKey =
                    mechanism.do_break(plaintextStream,
                                       std::istream(&hexDecoder).seekg(0));
            scoreLinePosAndKeyMap[scoreAndXorKey.first] =
                    std::make_tuple(lineNumber,
                                    startPos,
                                    scoreAndXorKey.second);
        } catch (std::runtime_error &) {
            continue;
        }
    }

    const auto & it = scoreLinePosAndKeyMap.rbegin();
    if (it == scoreLinePosAndKeyMap.rend())
        throw std::runtime_error("No winning line with single-byte XOR cipher");
    input.clear();
    if (input.seekg(std::get<1>(it->second), std::ios_base::beg)) {
        line_extract_streambuf lineExtractor(input);
        std::istream lineExtractStream(&lineExtractor);
        decode_hex_streambuf hexDecoder(lineExtractStream);
        mechanism.do_break(output, std::istream(&hexDecoder).seekg(0));
    } else {
        throw std::runtime_error("could not rewind input stream");
    }
    return std::make_tuple(std::get<0>(it->second),
                           it->first,
                           std::get<2>(it->second));
}

}  // close namespace cryptopals