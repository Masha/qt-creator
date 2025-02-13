// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
#pragma once

#include "itemnodedumper.h"

namespace QmlDesigner {
class Component;

class AssetNodeDumper : public ItemNodeDumper
{
public:
    AssetNodeDumper(const ModelNode &node);
    ~AssetNodeDumper() override = default;

    bool isExportable() const override;
    int priority() const override { return 200; }
    QJsonObject json(Component &component) const override;
};
}
