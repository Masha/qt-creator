// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "edit3dactions.h"

#include "edit3dview.h"
#include "indicatoractionwidget.h"
#include "seekerslider.h"

#include <qmldesignerconstants.h>
#include <qmldesignertr.h>

#include <utils3d.h>

#include <utils/algorithm.h>
#include <utils/qtcassert.h>

#include <QActionGroup>
#include <QMenu>

namespace QmlDesigner {

Edit3DActionTemplate::Edit3DActionTemplate(const QString &description,
                                           SelectionContextOperation action,
                                           Edit3DView *view,
                                           View3DActionType type)
    : DefaultAction(description)
    , m_action(action)
    , m_view(view)
    , m_type(type)
{}

void Edit3DActionTemplate::actionTriggered(bool b)
{
    if (m_type != View3DActionType::Empty && m_view->isAttached())
        m_view->model()->emitView3DAction(m_type, b);

    if (m_action)
        m_action(m_selectionContext);
}

Edit3DWidgetActionTemplate::Edit3DWidgetActionTemplate(QWidgetAction *widget,
                                                       SelectionContextOperation action)
    : PureActionInterface(widget)
    , m_action(action)
{
    QObject::connect(widget, &QAction::triggered, widget, [this](bool value) {
        actionTriggered(value);
    });
}

void Edit3DWidgetActionTemplate::setSelectionContext(const SelectionContext &selectionContext)
{
    m_selectionContext = selectionContext;
}

void Edit3DWidgetActionTemplate::actionTriggered([[maybe_unused]] bool b)
{
    if (m_action)
        m_action(m_selectionContext);
}

Edit3DSingleSelectionAction::Edit3DSingleSelectionAction(const QString &description,
                                                         const QList<Option> &options)
    : DefaultAction(description)
{
    QMenu *menu = new QMenu();
    QObject::connect(this, &QObject::destroyed, menu, &QObject::deleteLater);

    m_group = new QActionGroup(menu);

    for (const Option &option : options) {
        QAction *action = m_group->addAction(option.name);
        action->setCheckable(true);
        action->setData(option.data);
        action->setToolTip(option.tooltip);
        m_dataAction.insert(option.data, action);
    }

    connect(m_group, &QActionGroup::triggered, this, [&](QAction *action) {
        if (action)
            emit dataChanged(action->data().toByteArray());
    });

    menu->addActions(m_group->actions());
    setMenu(menu);
}

void Edit3DSingleSelectionAction::selectOption(const QByteArray &data)
{
    QAction *action = m_dataAction.value(data, nullptr);
    if (action)
        action->setChecked(true);
}

QByteArray Edit3DSingleSelectionAction::currentData() const
{
    QAction *action = m_group->checkedAction();
    if (action)
        return action->data().toByteArray();
    return {};
}

Edit3DAction::Edit3DAction(const QByteArray &menuId,
                           View3DActionType type,
                           const QString &description,
                           const QKeySequence &key,
                           bool checkable,
                           bool checked,
                           const QIcon &icon,
                           Edit3DView *view,
                           SelectionContextOperation selectionAction,
                           const QString &toolTip)
    : Edit3DAction(menuId, type, view, new Edit3DActionTemplate(description,
                                                                selectionAction,
                                                                view,
                                                                type))
{
    action()->setShortcut(key);
    action()->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    action()->setCheckable(checkable);
    action()->setChecked(checked);

    // Description will be used as tooltip by default if no explicit tooltip is provided
    if (!toolTip.isEmpty())
        action()->setToolTip(toolTip);

    action()->setIcon(icon);
}

Edit3DAction::Edit3DAction(const QByteArray &menuId,
                           View3DActionType type,
                           Edit3DView *view,
                           PureActionInterface *pureInt)
    : AbstractAction(pureInt)
    , m_menuId(menuId)
    , m_actionType(type)
{
    view->registerEdit3DAction(this);
}

QByteArray Edit3DAction::category() const
{
    return QByteArray();
}

View3DActionType Edit3DAction::actionType() const
{
    return m_actionType;
}

bool Edit3DAction::isVisible([[maybe_unused]] const SelectionContext &selectionContext) const
{
    return true;
}

bool Edit3DAction::isEnabled(const SelectionContext &selectionContext) const
{
    return isVisible(selectionContext);
}

Edit3DParticleSeekerAction::Edit3DParticleSeekerAction(const QByteArray &menuId,
                                                       View3DActionType type,
                                                       Edit3DView *view)
    : Edit3DAction(menuId,
                   type,
                   view,
                   new Edit3DWidgetActionTemplate(
                       new SeekerSliderAction(nullptr)))
{
    m_seeker = qobject_cast<SeekerSliderAction *>(action());
}

SeekerSliderAction *Edit3DParticleSeekerAction::seekerAction()
{
    return m_seeker;
}

bool Edit3DParticleSeekerAction::isVisible(const SelectionContext &) const
{
    return m_seeker->isVisible();
}

bool Edit3DParticleSeekerAction::isEnabled(const SelectionContext &) const
{
    return m_seeker->isEnabled();
}

Edit3DBakeLightsAction::Edit3DBakeLightsAction(const QIcon &icon,
                                               Edit3DView *view,
                                               SelectionContextOperation selectionAction)
    : Edit3DAction(QmlDesigner::Constants::EDIT3D_BAKE_LIGHTS,
                   View3DActionType::Empty,
                   Tr::tr("Bake Lights"),
                   QKeySequence(),
                   false,
                   false,
                   icon,
                   view,
                   selectionAction,
                   Tr::tr("Bake lights for the current 3D scene."))
    , m_view(view)
{

}

bool Edit3DBakeLightsAction::isVisible(const SelectionContext &) const
{
    return m_view->isBakingLightsSupported();
}

bool Edit3DBakeLightsAction::isEnabled(const SelectionContext &) const
{
    return m_view->isBakingLightsSupported()
            && !Utils3D::activeView3dId(m_view).isEmpty();
}

Edit3DIndicatorButtonAction::Edit3DIndicatorButtonAction(const QByteArray &menuId,
                                                         View3DActionType type,
                                                         const QString &description,
                                                         const QIcon &icon,
                                                         SelectionContextOperation customAction,
                                                         Edit3DView *view)
    : Edit3DAction(menuId,
                   type,
                   view,
                   new Edit3DWidgetActionTemplate(new IndicatorButtonAction(description, icon),
                                                  customAction))
{
    m_buttonAction = qobject_cast<IndicatorButtonAction *>(action());
}

void Edit3DIndicatorButtonAction::setIndicator(bool indicator)
{
    m_buttonAction->setIndicator(indicator);
}

bool Edit3DIndicatorButtonAction::isVisible(const SelectionContext &) const
{
    return m_buttonAction->isVisible();
}

bool Edit3DIndicatorButtonAction::isEnabled(const SelectionContext &) const
{
    return m_buttonAction->isEnabled();
}

Edit3DCameraViewAction::Edit3DCameraViewAction(const QByteArray &menuId,
                                               View3DActionType type,
                                               Edit3DView *view)
    : Edit3DAction(menuId,
                   type,
                   view,
                   new Edit3DSingleSelectionAction(tr("Camera view mode"), options()))
{
    Edit3DSingleSelectionAction *singleSelectionAction = qobject_cast<Edit3DSingleSelectionAction *>(
        action());
    QObject::connect(singleSelectionAction,
                     &Edit3DSingleSelectionAction::dataChanged,
                     view,
                     [this, view](const QByteArray &data) {
                         view->emitView3DAction(actionType(), data);
                     });
}

void Edit3DCameraViewAction::setMode(const QByteArray &mode)
{
    Edit3DSingleSelectionAction *singleSelectionAction = qobject_cast<Edit3DSingleSelectionAction *>(
        action());
    QTC_ASSERT(singleSelectionAction, return);
    if (mode.isEmpty())
        singleSelectionAction->selectOption("CameraOff");
    else
        singleSelectionAction->selectOption(mode);
}

QList<Edit3DSingleSelectionAction::Option> Edit3DCameraViewAction::options() const
{
    return {
        {tr("Hide Camera View"), tr("Never show the camera view."), "CameraOff"},
        {tr("Show Selected Camera View"),
         tr("Show the selected camera in the camera view."),
         "ShowSelectedCamera"},
        {tr("Always Show Camera View"),
         tr("Show the last selected camera in the camera view."),
         "AlwaysShowCamera"},
    };
}

} // namespace QmlDesigner
