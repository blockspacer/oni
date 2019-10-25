#pragma once

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

#define ONI_ENUM_DEF(NAME, COUNT, ...)                                                                              \
    namespace _detail_##NAME {class INIT_##NAME ;}                                                                  \
    struct NAME : public oni::EnumBase<COUNT, NAME> {                                                               \
    public:                                                                                                         \
       constexpr NAME(oni::i32 id_, oni::HashedString name_) noexcept : EnumBase(id_, name_) {}                     \
    private:                                                                                                        \
        friend _detail_##NAME::INIT_##NAME;                                                                         \
        friend std::array<NAME, COUNT>;                                                                             \
        constexpr NAME() noexcept {}                                                                                \
        constexpr NAME(bool) noexcept: EnumBase({ __VA_ARGS__ }) {}                                                 \
        };                                                                                                          \
        namespace _detail_##NAME { class INIT_##NAME { inline static NAME _STORAGE_FOR_##NAME = {true}; };}


namespace oni {
// TODO:
// 1) initialization is bullshit with the friend class and shit
// 2) fromString() function can return invalid data, I can't do compile-time check at the moment :( But maybe I don't want
// to? either compiling will be slow or run-time! But I guess I won't compile everything but I wil run every init
// doing run-time so might be better off with compile time?
// 3) Users of this need to use fromString() to get an instance, even for values that are known at compile time! :/
    template<i32 n, class _child>
    struct EnumBase {
        i32 id{0};
        HashedString name{"__INVALID__"};

        struct KeyVal {
            i32 id{0};
            HashedString name{"__INVALID__"};
        };

        constexpr
        EnumBase(i32 id_,
                 HashedString name_) noexcept : id(id_), name(name_) {}

        template<i32 N>
        static constexpr _child
        fromString(const c8 (&name_)[N]) {
            auto result = _child{0, HashedString(name_)};
            _init(result);
            return result;
        }

        constexpr bool
        operator==(const EnumBase &other) const {
            return other.id == id && other.name == name;
        }

        constexpr bool
        operator!=(const EnumBase &other) const {
            return other.id != id || other.name != name;
        }

        constexpr bool
        isValid() const {
            return id != 0 && name != invalid;
        }

        static auto
        begin() {
            return childStorage.begin();
        }

        static constexpr auto
        end() {
            return childStorage.end();
        }

        static constexpr i32
        size() {
            return n;
        }

        template<class Archive>
        void
        save(Archive &archive) const {
            archive(name.str);
        }

        template<class Archive>
        void
        load(Archive &archive) {
            std::string buffer{};
            archive(buffer);
            name = HashedString::view(buffer);
            // NOTE: After _init() hash will point to static storage so buffer is safe to go out of scope
            _init(name.hash);
        }

    protected:
        struct _KeyValRaw {
            i32 id{0};
            std::string_view str{};
        };

        constexpr
        EnumBase() = default;

        constexpr
        EnumBase(std::initializer_list<_KeyValRaw> map_) {
            assert(map_.size() <= n);

            storage[0].name = invalid;
            storage[0].id = 0;
            auto *iter = map_.begin();
            for (i32 i = 0; i < n; ++i) {
                storage[i + 1].name = HashedString(iter->str);
                storage[i + 1].id = iter->id;
                // TODO: this is basically what std::span is supposed to do.
                childStorage[i] = _child{storage[i + 1].id, storage[i + 1].name};
                ++iter;
            }
        }

    private:
        static constexpr void
        _init(_child &result) {
            for (i32 i = 1; i < n; ++i) {
                if (storage[i].name == result.name) {
                    result.name = storage[i].name;
                    result.id = storage[i].id;
                    return;
                }
            }
            assert(false);
            result.id = 0;
            result.name = invalid;
        }

        constexpr void
        _init(const Hash hash) {
            for (i32 i = 1; i < n; ++i) {
                if (storage[i].name.hash == hash) {
                    name = storage[i].name;
                    id = storage[i].id;
                    return;
                }
            }
            assert(false);
            id = 0;
            name = invalid;
        }

    private:
        static inline std::array<KeyVal, n + 1> storage;
        static inline std::array<_child, n> childStorage;
        static inline HashedString invalid = HashedString("__INVALID__");
    };
}