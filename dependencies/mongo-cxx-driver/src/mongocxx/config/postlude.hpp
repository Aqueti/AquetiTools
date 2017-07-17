// Copyright 2014 MongoDB Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// src/mongocxx/config/compiler.hpp
#undef MONGOCXX_INLINE
#pragma pop_macro("MONGOCXX_INLINE")
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#undef MONGOCXX_CALL
#pragma pop_macro("MONGOCXX_CALL")

// src/mongocxx/config/config.hpp.in
#undef MONGOCXX_INLINE_NAMESPACE_BEGIN
#pragma pop_macro("MONGOCXX_INLINE_NAMESPACE_BEGIN")
#undef MONGOCXX_INLINE_NAMESPACE_END
#pragma pop_macro("MONGOCXX_INLINE_NAMESPACE_END")

// src/mongocxx/config/version.hpp.in
#undef MONGOCXX_VERSION_EXTRA
#pragma pop_macro("MONGOCXX_VERSION_EXTRA")
#undef MONGOCXX_VERSION_MAJOR
#pragma pop_macro("MONGOCXX_VERSION_MAJOR")
#undef MONGOCXX_VERSION_MINOR
#pragma pop_macro("MONGOCXX_VERSION_MINOR")
#undef MONGOCXX_VERSION_PATCH
#pragma pop_macro("MONGOCXX_VERSION_PATCH")

// export.hpp (generated by cmake)
#undef MONGOCXX_API_H
#pragma pop_macro("MONGOCXX_API_H")
#undef MONGOCXX_API
#pragma pop_macro("MONGOCXX_API")
#undef MONGOCXX_PRIVATE
#pragma pop_macro("MONGOCXX_PRIVATE")
#undef MONGOCXX_DEPRECATED
#pragma pop_macro("MONGOCXX_DEPRECATED")
#undef MONGOCXX_DEPRECATED_EXPORT
#pragma pop_macro("MONGOCXX_DEPRECATED_EXPORT")
#undef MONGOCXX_DEPRECATED_NO_EXPORT
#pragma pop_macro("MONGOCXX_DEPRECATED_NO_EXPORT")
#undef DEFINE_NO_DEPRECATED
#pragma pop_macro("DEFINE_NO_DEPRECATED")
#undef MONGOCXX_NO_DEPRECATED
#pragma pop_macro("MONGOCXX_NO_DEPRECATED")

// prelude.hpp
#undef MONGOCXX_UNREACHABLE
#pragma pop_macro("MONGOCXX_UNREACHABLE")
