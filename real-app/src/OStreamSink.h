#pragma once

// Work-around for compilation error caused by spdlog using Windows headers
#include <Windows.h>

#include <spdlog/sinks/base_sink.h>

#include <mutex>

namespace miniant::Spdlog {

class OStreamSink : public spdlog::sinks::base_sink<std::mutex> {
public:
    explicit OStreamSink(std::shared_ptr<std::ostream> outputStream, bool forceFlush=false);

    std::mutex& GetMutex();

protected:
    void sink_it_(const spdlog::details::log_msg& message) override;
    void flush_() override;

private:
    std::shared_ptr<std::ostream> m_outputStream;
    bool m_forceFlush;
};

}
