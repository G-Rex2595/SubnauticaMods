#pragma once
#include "..\stdafx.h"
//#include <mono\metadata\debug-helpers.h>

namespace MonoTypes {
   typedef MonoThread*(__cdecl* mono_thread_attach_t)(MonoDomain* pMonoDomain);
   typedef MonoClass*(__cdecl* mono_class_from_name_t)(MonoImage* pMonoImage, const char* _namespace, const char* name);
   typedef MonoMethod*(__cdecl* mono_class_get_method_from_name_t)(MonoClass* pMonoClass, const char* name, int param_count);
   typedef MonoObject*(__cdecl* mono_runtime_invoke_t)(MonoMethod* pMonoMethod, LPVOID pInstancePointer, void** params, MonoObject** exc);
   typedef MonoAssembly*(__cdecl* mono_domain_assembly_open_t)(MonoDomain* pMonoDomain, const char* pathFile);
   typedef MonoImage*(__cdecl* mono_assembly_get_image_t)(MonoAssembly* pMonoAssembly);
   //typedef MonoMethodDesc*(__cdecl* mono_method_desc_new_t)(const char* name, bool include_namespace);
   //typedef MonoMethod*(__cdecl* mono_method_desc_search_in_image_t)(MonoMethodDesc* desc, MonoImage* image);
   typedef MonoDomain*(__cdecl* mono_root_domain_get_t)();
   typedef MonoDomain*(__cdecl* mono_domain_get_t)();
   typedef MonoString*(__cdecl* mono_string_new_t)(MonoDomain* pMonoDomain, const char* text);

   template <typename T>
   T GetMonoFunction(HMODULE hMono, const char* fnName) {
      return reinterpret_cast<T>(GetProcAddress(hMono, fnName));
   }
}