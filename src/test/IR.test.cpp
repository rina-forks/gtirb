#include <gtest/gtest.h>
#include <gtirb/IR.hpp>
#include <gtirb/Module.hpp>
#include <memory>

TEST(Unit_IR, ctor_0)
{
    EXPECT_NO_THROW(gtirb::IR());
}

TEST(Unit_IR, getMainModule)
{
    auto ir = gtirb::IR();
    EXPECT_TRUE(ir.getMainModule() == nullptr);

    auto main = ir.getOrCreateMainModule();
    EXPECT_TRUE(main != nullptr);

    EXPECT_EQ(main, ir.getMainModule());
}

TEST(Unit_IR, getOrCreateMainModule)
{
    auto ir = gtirb::IR();
    EXPECT_TRUE(ir.getMainModule() == nullptr);

    EXPECT_NO_THROW(ir.getOrCreateMainModule());
    auto main = ir.getOrCreateMainModule();
    EXPECT_TRUE(main != nullptr);
}

TEST(Unit_IR, getModulesWithPreferredEA)
{
    const gtirb::EA preferredEA{22678};
    const size_t modulesWithEA{3};
    const size_t modulesWithoutEA{5};

    auto ir = gtirb::IR();

    for(size_t i = 0; i < modulesWithEA; ++i)
    {
        auto m = std::make_unique<gtirb::Module>();
        m->setPreferredEA(preferredEA);
        EXPECT_NO_THROW(ir.push_back(std::move(m)));
    }

    for(size_t i = 0; i < modulesWithoutEA; ++i)
    {
        auto m = std::make_unique<gtirb::Module>();
        EXPECT_NO_THROW(ir.push_back(std::move(m)));
    }

    EXPECT_EQ(modulesWithEA + modulesWithoutEA, ir.size());
    const auto modules = ir.getModulesWithPreferredEA(preferredEA);
    EXPECT_FALSE(modules.empty());
    EXPECT_EQ(modulesWithEA, modules.size());
}

TEST(Unit_IR, getModulesContainingEA)
{
    const gtirb::EA ea{22678};
    const gtirb::EA eaOffset{2112};

    auto ir = gtirb::IR();

    // EA at lower bound
    {
        auto m = std::make_unique<gtirb::Module>();
        m->setEAMinMax({ea, ea + eaOffset});
        EXPECT_NO_THROW(ir.push_back(std::move(m)));
    }

    // EA inside range
    {
        auto m = std::make_unique<gtirb::Module>();
        m->setEAMinMax({ea - eaOffset, ea + eaOffset});
        EXPECT_NO_THROW(ir.push_back(std::move(m)));
    }

    // EA at max (should not be returned)
    {
        auto m = std::make_unique<gtirb::Module>();
        m->setEAMinMax({ea - eaOffset, ea});
        EXPECT_NO_THROW(ir.push_back(std::move(m)));
    }

    EXPECT_EQ(size_t(3), ir.size());
    const auto modules = ir.getModulesContainingEA(ea);
    EXPECT_FALSE(modules.empty());
    EXPECT_EQ(size_t(2), modules.size());
}
