#pragma once

#include "RE/Skyrim.h"

namespace Events {
    class MenuOpenCloseEventHandler : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
    public:
        using EventResult = RE::BSEventNotifyControl;

        static MenuOpenCloseEventHandler* GetSingleton();

        virtual EventResult ProcessEvent(const RE::MenuOpenCloseEvent* a_event,
                                         RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_eventSource) override;

        void SetBethTranslator(RE::GPtr<RE::GFxTranslator>&& a_translator);

    private:
        MenuOpenCloseEventHandler() = default;
        MenuOpenCloseEventHandler(const MenuOpenCloseEventHandler&) = delete;
        MenuOpenCloseEventHandler(MenuOpenCloseEventHandler&&) = delete;

        virtual ~MenuOpenCloseEventHandler();

        MenuOpenCloseEventHandler& operator=(const MenuOpenCloseEventHandler&) = delete;
        MenuOpenCloseEventHandler& operator=(MenuOpenCloseEventHandler&&) = delete;

        RE::GPtr<RE::GFxTranslator> _bethImpl;
    };
}  // namespace Events