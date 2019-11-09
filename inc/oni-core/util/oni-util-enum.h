#pragma once

#include <functional>

#include <oni-core/common/oni-common-typedef.h>
#include <oni-core/util/oni-util-hash.h>
#include <oni-core/math/oni-math-function.h>

/* Criteria for a better enum type:
 * 1) minimal macro and template use
 * 2) simple io from and to string
 * 3) possibility to iterate over
 * 4) header only
 * 5) storage should require ~64-bit
 * 6) ::ENUM_NAME or something quite similar should be available on the enum
 * 7) easy to use in if statements
 */


#define ONI_ENUM_DEF(NAME, ...)                                                                                         \
    namespace detail{ namespace {inline static const constexpr oni::Enum storage_##NAME [] = { __VA_ARGS__};}}          \
    struct NAME : public oni::detail::EnumBase<                                                                         \
            sizeof(detail::storage_##NAME) / sizeof(oni::Enum), detail::storage_##NAME> {                               \
       template<oni::i32 n>                                                                                             \
       inline static constexpr                                                                                          \
       NAME GET(const oni::c8 (&name)[n])                                                                               \
       { return {_get(name)};}                                                                                          \
       inline constexpr bool operator==(const oni::HashedString &other) const { return other.hash == name.hash; }       \
       inline constexpr bool operator!=(const oni::HashedString &other) const { return other.hash != name.hash; }       \
};

namespace oni {
    struct Enum {
        oni::i32 id{};
        oni::HashedString name{};
    };

    namespace detail {
        template<oni::i32 N, auto v>
        struct EnumBase : public Enum {
            inline static constexpr auto
            begin() {
                return storage;
            }

            inline static constexpr auto
            end() {
                return storage + N;
            }

            inline constexpr bool
            operator==(const EnumBase &other) const {
                return other.id == id && other.name == name;
            }

            inline constexpr bool
            operator!=(const EnumBase &other) const {
                return other.id != id || other.name != name;
            }

            inline static constexpr oni::i32
            size() {
                return N;
            }

            // NOTE: Implicit so that this class can be used in switch statements
            inline constexpr operator i32() const {
                return id;
            }

            template<class Out, class Adaptor>
            static inline Out *
            adapt(Adaptor adaptor) {
                static bool init = true;
                static auto result = std::array<Out, N>();
                if (init) {
                    for (oni::i32 i = 0; i < N; ++i) {
                        result[i] = adaptor(storage[i]);
                    }
                    init = false;
                }
                return result.data();
            }

            // TODO: Not super sure about this exposed here. Do another pass on the interface
            void
            _runtimeInit(const oni::Hash hash) {
                for (oni::i32 i = 0; i < N; ++i) {
                    if (storage[i].name.hash == hash) {
                        name = storage[i].name;
                        id = storage[i].id;
                        return;
                    }
                }
                assert(false);
                id = INVALID.id;
                name = INVALID.name;
            }

        protected:
            template<oni::i32 n>
            static inline constexpr Enum
            _get(const oni::c8 (&name_)[n]) {
                return _find(oni::HashedString::makeHashFromLiteral(name_), 0, storage);
            }

        private:
            // TODO: How can I make this better, the diagnosis error is misleading
            inline static constexpr Enum
            _notFound() {
                throw std::logic_error("Invalid value specified for the enum");
                return INVALID;
            }

            inline static constexpr Enum
            _find(const oni::Hash hash,
                  oni::i32 i,
                  const Enum *candidate) {
                return (candidate->name.hash == hash) ? *candidate :
                       ((i == N) ? _notFound() : _find(hash, i + 1, candidate + 1));
            }

        private:
            static inline const constexpr Enum *storage = v;
            static inline const constexpr Enum INVALID = {0, "__INVALID__"};
        };
    }
}

namespace std {
    template<>
    class hash<oni::Enum> {
    public:
        constexpr size_t
        operator()(oni::Enum const &hs) const {
            return hs.name.hash.value;
        }

        constexpr bool
        operator()(const oni::Enum &lhs,
                   const oni::Enum &rhs) const {
            return lhs.name.hash == rhs.name.hash;
        }
    };
}