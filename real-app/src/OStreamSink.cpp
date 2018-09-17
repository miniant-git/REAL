#include "OStreamSink.h"

using namespace miniant::Spdlog;

OStreamSink::OStreamSink(std::shared_ptr<std::ostream> outputStream, bool forceFlush):
    m_outputStream(std::move(outputStream)),
    m_forceFlush(forceFlush) {}

std::mutex& OStreamSink::GetMutex() {
    return mutex_;
}

void OStreamSink::sink_it_(const spdlog::details::log_msg& message) {
    fmt::memory_buffer formattedMessage;
    sink::formatter_->format(message, formattedMessage);
    m_outputStream->write(formattedMessage.data(), static_cast<std::streamsize>(formattedMessage.size()));
    if (m_forceFlush)
        m_outputStream->flush();
}

void OStreamSink::flush_() {
    m_outputStream->flush();
}
