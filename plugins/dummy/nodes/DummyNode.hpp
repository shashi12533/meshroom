#pragma once

#include <dglib/dg.hpp>

class DummyNode : public dg::Node
{
public:
    DummyNode(std::string nodeName);

public:
    std::vector<dg::Command> prepare(dg::Cache&, bool&) override;
    void compute(const std::vector<std::string>& args) const override {}
    std::string type() const override { return "DummyNode"; }
};
