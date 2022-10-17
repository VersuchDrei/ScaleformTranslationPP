#include <stddef.h>

#include "Events.h"
#include "Hooks.h"

using namespace RE::BSScript;
using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

namespace {
    void InitializeLogging() {
        auto path = log_directory();
        if (!path) {
            report_and_fail("Unable to lookup SKSE logs directory.");
        }
        *path /= PluginDeclaration::GetSingleton()->GetName();
        *path += L".log";

        std::shared_ptr<spdlog::logger> log;
        if (IsDebuggerPresent()) {
            log = std::make_shared<spdlog::logger>(
                "Global", std::make_shared<spdlog::sinks::msvc_sink_mt>());
        } else {
            log = std::make_shared<spdlog::logger>(
                "Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));
        }
        log->set_level(spdlog::level::info);
        log->flush_on(spdlog::level::info);

        spdlog::set_default_logger(std::move(log));
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] %v");
    }

    void MessageHandler(SKSE::MessagingInterface::Message* a_msg) {
        switch (a_msg->type) {
            case SKSE::MessagingInterface::kInputLoaded: {
                const auto ui = RE::UI::GetSingleton();
                const auto menuSink = Events::MenuOpenCloseEventHandler::GetSingleton();
                ui->AddEventSink(menuSink);

                Hooks::Install();
            } break;
        }
    }
}

SKSEPluginLoad(const LoadInterface* skse) {
    InitializeLogging();

    auto* plugin = PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();
    log::info("{} {} is loading...", plugin->GetName(), version);


    Init(skse);
    SKSE::AllocTrampoline(1 << 4);

    const auto messaging = SKSE::GetMessagingInterface();
    messaging->RegisterListener("SKSE", MessageHandler);

    log::info("{} has finished loading.", plugin->GetName());
    return true;
}
