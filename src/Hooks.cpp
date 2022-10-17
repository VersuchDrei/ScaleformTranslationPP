#include "Hooks.h"

#include "Events.h"
#include "LocaleManager.h"

namespace Hooks {
    namespace {
        class BSScaleformMovieLoaderEx : public RE::BSScaleformManager {
        public:
            BSScaleformMovieLoaderEx* Ctor() {
                _ctor(this);

                const auto bethTranslator =
                    loader->GetState<RE::BSScaleformTranslator>(RE::GFxState::StateType::kTranslator);

                const auto newTranslator = new LocaleManager(bethTranslator->translator);
                loader->SetState(RE::GFxState::StateType::kTranslator, newTranslator);

                const auto menuSink = Events::MenuOpenCloseEventHandler::GetSingleton();
                menuSink->SetBethTranslator(std::move(bethTranslator));

                return this;
            }

            static void InstallHooks() {
                int offset = REL::Module::GetRuntime() == REL::Module::Runtime::AE ? 0xE91 : 0xA08;
                REL::Relocation<std::uintptr_t> target{RELOCATION_ID(35548, 36547), offset};
                auto& trampoline = SKSE::GetTrampoline();
                _ctor = trampoline.write_call<5>(target.address(), &BSScaleformMovieLoaderEx::Ctor);
                logger::debug("Installed hooks for ({})", typeid(BSScaleformMovieLoaderEx).name());
            }

        private:
            using Ctor_t = decltype(&BSScaleformMovieLoaderEx::Ctor);
            static inline REL::Relocation<Ctor_t> _ctor;
        };
    }  // namespace

    void Install() {
        BSScaleformMovieLoaderEx::InstallHooks();
        logger::debug("Installed all hooks");
    }
}  // namespace Hooks