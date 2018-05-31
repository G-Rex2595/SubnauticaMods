#include "stdafx.h"
#include "Helpers\MonoTypes.h"
#include "Helpers\DetourHelper.hpp"
#include <json.hpp>
using json = nlohmann::json;

namespace ModLoader {
   MonoTypes::mono_thread_attach_t              mono_thread_attach;
   MonoTypes::mono_class_from_name_t            mono_class_from_name;
   MonoTypes::mono_class_get_method_from_name_t mono_class_get_method_from_name;
   MonoTypes::mono_runtime_invoke_t             mono_runtime_invoke;
   MonoTypes::mono_domain_assembly_open_t       mono_domain_assembly_open;
   MonoTypes::mono_assembly_get_image_t         mono_assembly_get_image;

   DetourHelper64 detourHelper = {};
   MonoDomain* WINAPI hkMonoGetDomain() {
      MonoDomain* pMonoDomain = nullptr;
      {
         auto& detourInfo = detourHelper.detouredFunctions["mono_domain_get"];
         detourInfo.restore();
         pMonoDomain = ((MonoTypes::mono_domain_get_t)detourInfo.origAddress)();
         if (!pMonoDomain) {
            std::cout << "[ModLoader] MonoDomain is null!!!" << std::endl;
            return nullptr; // crash on purpose
         }
         detourHelper.~DetourHelper64();
      }

      std::cout << "[QMods Compatibility] Loading QMods-dependent mods" << std::endl;
      for (auto& modEntry : std::filesystem::directory_iterator("QMods")) {
         if (modEntry.is_directory()) {
            std::string modName = modEntry.path().stem().u8string();
            std::cout << "[QMods Compatibility] Loading mod '" + modName + "'" << std::endl;
            for (auto& file : std::filesystem::directory_iterator(modEntry))
            {
               if (file.path().filename().u8string() == std::string("mod.json")) {
                  std::ifstream i(file);
                  if (!i)
                     continue;
                  json j;
                  try
                  {
                     i >> j;
                  } catch (const json::exception& exc)
                  {
                     std::cout << "[QMods Compatibility] Exception occured while parsing 'mod.json' for mod '" + modName + "'!" << std::endl;
                     continue;
                  }

                  if (j.count("EntryMethod") > 0) {
                     std::string monoStrings[3];

                     size_t pos = 0;
                     std::string strEntryMethod(j["EntryMethod"].get<std::string>());
                     for (size_t i = 0; i < 3; i++)
                     {
                        pos = strEntryMethod.find(".");
                        if (pos == std::string::npos) {
                           monoStrings[i] = strEntryMethod;
                           break;
                        }

                        monoStrings[i] = strEntryMethod.substr(0, pos);
                        strEntryMethod.erase(0, pos + 1);
                     }

                     if (j.count("AssemblyName") > 0) {
                        std::string   _dll        = std::filesystem::absolute(modEntry.path() / j["AssemblyName"].get<std::string>()).u8string();
                        MonoAssembly* modAssembly = mono_domain_assembly_open(pMonoDomain, _dll.c_str());
                        if (modAssembly) {
                           MonoImage*  pMonoImage    = mono_assembly_get_image(modAssembly);
                           MonoClass*  pTargetClass  = mono_class_from_name(pMonoImage, monoStrings[0].c_str(), monoStrings[1].c_str());
                           MonoMethod* pTargetMethod = mono_class_get_method_from_name(pTargetClass, monoStrings[2].c_str(), 0);
                           mono_runtime_invoke(pTargetMethod, nullptr, nullptr, nullptr);
                           std::cout << "[QMods Compatibility] '" + modName + "' was successfully loaded." << std::endl;
                        }
                     }
                  } else {
                     std::cout << "[QMods Compatibility] No EntryMethod!!" << std::endl;
                     continue;
                  }
               }
            }
         }
      }

      return pMonoDomain;
   }

   DWORD WINAPI Init(LPVOID) {
      AllocConsole();
      freopen("ModLoader_out.txt", "w", stdout);
      freopen("CONOUT$", "w", stdout);

      HMODULE hMono = nullptr;
      while (!hMono) {
         hMono = GetModuleHandleA("mono.dll");
         Sleep(100);
      }
      mono_thread_attach              = MonoTypes::GetMonoFunction<MonoTypes::mono_thread_attach_t>(hMono, "mono_thread_attach");
      mono_class_from_name            = MonoTypes::GetMonoFunction<MonoTypes::mono_class_from_name_t>(hMono, "mono_class_from_name");
      mono_class_get_method_from_name = MonoTypes::GetMonoFunction<MonoTypes::mono_class_get_method_from_name_t>(hMono, "mono_class_get_method_from_name");
      mono_runtime_invoke             = MonoTypes::GetMonoFunction<MonoTypes::mono_runtime_invoke_t>(hMono, "mono_runtime_invoke");
      mono_domain_assembly_open       = MonoTypes::GetMonoFunction<MonoTypes::mono_domain_assembly_open_t>(hMono, "mono_domain_assembly_open");
      mono_assembly_get_image         = MonoTypes::GetMonoFunction<MonoTypes::mono_assembly_get_image_t>(hMono, "mono_assembly_get_image");

      auto mono_domain_get = MonoTypes::GetMonoFunction<MonoTypes::mono_domain_get_t>(hMono, "mono_domain_get");
      detourHelper.detour("mono_domain_get", (DWORD64)mono_domain_get, (DWORD64)&ModLoader::hkMonoGetDomain);

      return TRUE;
   }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID /*lpReserved*/)
{
   if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
      DisableThreadLibraryCalls(hModule);
      CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ModLoader::Init, NULL, 0, NULL);
   }
   return TRUE;
}