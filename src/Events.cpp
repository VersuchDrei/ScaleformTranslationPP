#include "Events.h"

namespace Events {
    MenuOpenCloseEventHandler* MenuOpenCloseEventHandler::GetSingleton() {
        static MenuOpenCloseEventHandler singleton;
        return &singleton;
    }

    auto MenuOpenCloseEventHandler::ProcessEvent(
        const RE::MenuOpenCloseEvent* a_event,
        [[maybe_unused]] RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_eventSource) -> EventResult {
        auto uiStr = RE::InterfaceStrings::GetSingleton();
        if (a_event && a_event->opening && a_event->menuName == uiStr->journalMenu) {
            const auto ui = RE::UI::GetSingleton();
            const auto journal = ui->GetMenu(uiStr->journalMenu);
            if (journal && journal->uiMovie) {
                journal->uiMovie->SetState(RE::GFxState::StateType::kTranslator, _bethImpl.get());
            }
        }

        return EventResult::kContinue;
    }

    void MenuOpenCloseEventHandler::SetBethTranslator(RE::GPtr<RE::GFxTranslator>&& a_translator) {
        _bethImpl = std::move(a_translator);
    }

    MenuOpenCloseEventHandler::~MenuOpenCloseEventHandler() {
        util::memzero(std::addressof(_bethImpl));  // avoid crash in static dtor
    }
}  // namespace Events