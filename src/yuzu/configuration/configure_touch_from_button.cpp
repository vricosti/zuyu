// SPDX-FileCopyrightText: 2020 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>

#include "common/param_package.h"
#include "common/settings.h"
#include "core/frontend/framebuffer_layout.h"
#include "input_common/main.h"
#include "yuzu/configuration/configure_touch_from_button.h"
#include "yuzu/configuration/configure_touch_widget.h"

static QString GetKeyName(int key_code) {
    switch (key_code) {
    case Qt::Key_Shift:
        return QObject::tr("Shift");
    case Qt::Key_Control:
        return QObject::tr("Ctrl");
    case Qt::Key_Alt:
        return QObject::tr("Alt");
    case Qt::Key_Meta:
        return QString{};
    default:
        return QKeySequence(key_code).toString();
    }
}

static QString ButtonToText(const Common::ParamPackage& param) {
    if (!param.Has("engine")) {
        return QObject::tr("[not set]");
    }

    if (param.Get("engine", "") == "keyboard") {
        return GetKeyName(param.Get("code", 0));
    }

    if (param.Get("engine", "") == "sdl") {
        if (param.Has("hat")) {
            const QString hat_str = QString::fromStdString(param.Get("hat", ""));
            const QString direction_str = QString::fromStdString(param.Get("direction", ""));

            return QObject::tr("Hat %1 %2").arg(hat_str, direction_str);
        }

        if (param.Has("axis")) {
            const QString axis_str = QString::fromStdString(param.Get("axis", ""));
            const QString direction_str = QString::fromStdString(param.Get("direction", ""));

            return QObject::tr("Axis %1%2").arg(axis_str, direction_str);
        }

        if (param.Has("button")) {
            const QString button_str = QString::fromStdString(param.Get("button", ""));

            return QObject::tr("Button %1").arg(button_str);
        }

        return {};
    }

    return QObject::tr("[unknown]");
}

ConfigureTouchFromButton::ConfigureTouchFromButton(
    QWidget* parent, const std::vector<Settings::TouchFromButtonMap>& touch_maps_,
    InputCommon::InputSubsystem* input_subsystem_, const int default_index)
    : QDialog(parent), touch_maps{touch_maps_}, input_subsystem{input_subsystem_},
      selected_index{default_index}, timeout_timer(std::make_unique<QTimer>()),
      poll_timer(std::make_unique<QTimer>()) {
    setWindowTitle(tr("Configure Touchscreen Mappings"));
    resize(500, 500);

    auto* main_layout = new QVBoxLayout(this);

    // Top: Mapping combo + buttons
    auto* top_layout = new QHBoxLayout;
    top_layout->addWidget(new QLabel(tr("Mapping:"), this));

    mapping = new QComboBox(this);
    mapping->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    top_layout->addWidget(mapping);

    button_new = new QPushButton(tr("New"), this);
    button_new->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    top_layout->addWidget(button_new);

    button_delete = new QPushButton(tr("Delete"), this);
    button_delete->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    top_layout->addWidget(button_delete);

    button_rename = new QPushButton(tr("Rename"), this);
    button_rename->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    top_layout->addWidget(button_rename);

    main_layout->addLayout(top_layout);

    // Separator
    auto* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    main_layout->addWidget(line);

    // Instructions + Delete Point
    auto* instructions_layout = new QHBoxLayout;
    auto* instructions_label = new QLabel(
        tr("Click the bottom area to add a point, then press a button to bind.\n"
           "Drag points to change position, or double-click table cells to edit values."),
        this);
    instructions_label->setTextFormat(Qt::PlainText);
    instructions_layout->addWidget(instructions_label);
    instructions_layout->addStretch();

    button_delete_bind = new QPushButton(tr("Delete Point"), this);
    instructions_layout->addWidget(button_delete_bind);

    main_layout->addLayout(instructions_layout);

    // Binding list
    binding_list = new QTreeView(this);
    binding_list->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    binding_list->setRootIsDecorated(false);
    binding_list->setUniformRowHeights(true);
    binding_list->setItemsExpandable(false);
    main_layout->addWidget(binding_list);

    // Touch screen preview
    bottom_screen = new TouchScreenPreview(this);
    bottom_screen->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    bottom_screen->setMinimumSize(160, 120);
    bottom_screen->setBaseSize(320, 240);
    bottom_screen->setCursor(Qt::CrossCursor);
    bottom_screen->setMouseTracking(true);
    bottom_screen->setAutoFillBackground(true);
    bottom_screen->setFrameShape(QFrame::StyledPanel);
    bottom_screen->setFrameShadow(QFrame::Sunken);
    main_layout->addWidget(bottom_screen);

    // Bottom: coord label + button box
    auto* bottom_layout = new QHBoxLayout;
    coord_label = new QLabel(this);
    coord_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    coord_label->setTextFormat(Qt::PlainText);
    bottom_layout->addWidget(coord_label);

    button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    bottom_layout->addWidget(button_box);

    main_layout->addLayout(bottom_layout);

    // Setup model
    binding_list_model = new QStandardItemModel(0, 3, this);
    binding_list_model->setHorizontalHeaderLabels(
        {tr("Button"), tr("X", "X axis"), tr("Y", "Y axis")});
    binding_list->setModel(binding_list_model);
    bottom_screen->SetCoordLabel(coord_label);

    SetConfiguration();
    UpdateUiDisplay();
    ConnectEvents();
}

ConfigureTouchFromButton::~ConfigureTouchFromButton() = default;

void ConfigureTouchFromButton::showEvent(QShowEvent* ev) {
    QWidget::showEvent(ev);

    const int w =
        binding_list->viewport()->contentsRect().width() / binding_list_model->columnCount();
    if (w <= 0) {
        return;
    }
    binding_list->setColumnWidth(0, w);
    binding_list->setColumnWidth(1, w);
    binding_list->setColumnWidth(2, w);
}

void ConfigureTouchFromButton::SetConfiguration() {
    for (const auto& touch_map : touch_maps) {
        mapping->addItem(QString::fromStdString(touch_map.name));
    }

    mapping->setCurrentIndex(selected_index);
}

void ConfigureTouchFromButton::UpdateUiDisplay() {
    button_delete->setEnabled(touch_maps.size() > 1);
    button_delete_bind->setEnabled(false);

    binding_list_model->removeRows(0, binding_list_model->rowCount());

    for (const auto& button_str : touch_maps[selected_index].buttons) {
        Common::ParamPackage package{button_str};
        QStandardItem* button = new QStandardItem(ButtonToText(package));
        button->setData(QString::fromStdString(button_str));
        button->setEditable(false);
        QStandardItem* xcoord = new QStandardItem(QString::number(package.Get("x", 0)));
        QStandardItem* ycoord = new QStandardItem(QString::number(package.Get("y", 0)));
        binding_list_model->appendRow({button, xcoord, ycoord});

        const int dot = bottom_screen->AddDot(package.Get("x", 0), package.Get("y", 0));
        button->setData(dot, DataRoleDot);
    }
}

void ConfigureTouchFromButton::ConnectEvents() {
    connect(mapping, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index) {
        SaveCurrentMapping();
        selected_index = index;
        UpdateUiDisplay();
    });
    connect(button_new, &QPushButton::clicked, this, &ConfigureTouchFromButton::NewMapping);
    connect(button_delete, &QPushButton::clicked, this,
            &ConfigureTouchFromButton::DeleteMapping);
    connect(button_rename, &QPushButton::clicked, this,
            &ConfigureTouchFromButton::RenameMapping);
    connect(button_delete_bind, &QPushButton::clicked, this,
            &ConfigureTouchFromButton::DeleteBinding);
    connect(binding_list, &QTreeView::doubleClicked, this,
            &ConfigureTouchFromButton::EditBinding);
    connect(binding_list->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &ConfigureTouchFromButton::OnBindingSelection);
    connect(binding_list_model, &QStandardItemModel::itemChanged, this,
            &ConfigureTouchFromButton::OnBindingChanged);
    connect(binding_list->model(), &QStandardItemModel::rowsAboutToBeRemoved, this,
            &ConfigureTouchFromButton::OnBindingDeleted);
    connect(bottom_screen, &TouchScreenPreview::DotAdded, this,
            &ConfigureTouchFromButton::NewBinding);
    connect(bottom_screen, &TouchScreenPreview::DotSelected, this,
            &ConfigureTouchFromButton::SetActiveBinding);
    connect(bottom_screen, &TouchScreenPreview::DotMoved, this,
            &ConfigureTouchFromButton::SetCoordinates);
    connect(button_box, &QDialogButtonBox::accepted, this,
            &ConfigureTouchFromButton::ApplyConfiguration);
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(timeout_timer.get(), &QTimer::timeout, [this]() { SetPollingResult({}, true); });

    connect(poll_timer.get(), &QTimer::timeout, [this]() {
        const auto& params = input_subsystem->GetNextInput();
        if (params.Has("engine")) {
            SetPollingResult(params, false);
            return;
        }
    });
}

void ConfigureTouchFromButton::SaveCurrentMapping() {
    auto& map = touch_maps[selected_index];
    map.buttons.clear();
    for (int i = 0, rc = binding_list_model->rowCount(); i < rc; ++i) {
        const auto bind_str = binding_list_model->index(i, 0)
                                  .data(Qt::ItemDataRole::UserRole + 1)
                                  .toString()
                                  .toStdString();
        if (bind_str.empty()) {
            continue;
        }
        Common::ParamPackage params{bind_str};
        if (!params.Has("engine")) {
            continue;
        }
        params.Set("x", binding_list_model->index(i, 1).data().toInt());
        params.Set("y", binding_list_model->index(i, 2).data().toInt());
        map.buttons.emplace_back(params.Serialize());
    }
}

void ConfigureTouchFromButton::NewMapping() {
    const QString name =
        QInputDialog::getText(this, tr("New Profile"), tr("Enter the name for the new profile."));
    if (name.isEmpty()) {
        return;
    }
    touch_maps.emplace_back(Settings::TouchFromButtonMap{name.toStdString(), {}});
    mapping->addItem(name);
    mapping->setCurrentIndex(mapping->count() - 1);
}

void ConfigureTouchFromButton::DeleteMapping() {
    const auto answer = QMessageBox::question(
        this, tr("Delete Profile"), tr("Delete profile %1?").arg(mapping->currentText()));
    if (answer != QMessageBox::Yes) {
        return;
    }
    const bool blocked = mapping->blockSignals(true);
    mapping->removeItem(selected_index);
    mapping->blockSignals(blocked);
    touch_maps.erase(touch_maps.begin() + selected_index);
    selected_index = mapping->currentIndex();
    UpdateUiDisplay();
}

void ConfigureTouchFromButton::RenameMapping() {
    const QString new_name = QInputDialog::getText(this, tr("Rename Profile"), tr("New name:"));
    if (new_name.isEmpty()) {
        return;
    }
    mapping->setItemText(selected_index, new_name);
    touch_maps[selected_index].name = new_name.toStdString();
}

void ConfigureTouchFromButton::GetButtonInput(const int row_index, const bool is_new) {
    if (timeout_timer->isActive()) {
        return;
    }
    binding_list_model->item(row_index, 0)->setText(tr("[press key]"));

    input_setter = [this, row_index, is_new](const Common::ParamPackage& params,
                                             const bool cancel) {
        auto* cell = binding_list_model->item(row_index, 0);
        if (cancel) {
            if (is_new) {
                binding_list_model->removeRow(row_index);
            } else {
                cell->setText(
                    ButtonToText(Common::ParamPackage{cell->data().toString().toStdString()}));
            }
        } else {
            cell->setText(ButtonToText(params));
            cell->setData(QString::fromStdString(params.Serialize()));
        }
    };

    input_subsystem->BeginMapping(InputCommon::Polling::InputType::Button);

    grabKeyboard();
    grabMouse();
    qApp->setOverrideCursor(QCursor(Qt::CursorShape::ArrowCursor));
    timeout_timer->start(5000);
    poll_timer->start(200);
}

void ConfigureTouchFromButton::NewBinding(const QPoint& pos) {
    auto* button = new QStandardItem();
    button->setEditable(false);
    auto* x_coord = new QStandardItem(QString::number(pos.x()));
    auto* y_coord = new QStandardItem(QString::number(pos.y()));

    const int dot_id = bottom_screen->AddDot(pos.x(), pos.y());
    button->setData(dot_id, DataRoleDot);

    binding_list_model->appendRow({button, x_coord, y_coord});
    binding_list->setFocus();
    binding_list->setCurrentIndex(button->index());

    GetButtonInput(binding_list_model->rowCount() - 1, true);
}

void ConfigureTouchFromButton::EditBinding(const QModelIndex& qi) {
    if (qi.row() >= 0 && qi.column() == 0) {
        GetButtonInput(qi.row(), false);
    }
}

void ConfigureTouchFromButton::DeleteBinding() {
    const int row_index = binding_list->currentIndex().row();
    if (row_index < 0) {
        return;
    }
    bottom_screen->RemoveDot(binding_list_model->index(row_index, 0).data(DataRoleDot).toInt());
    binding_list_model->removeRow(row_index);
}

void ConfigureTouchFromButton::OnBindingSelection(const QItemSelection& selected,
                                                  const QItemSelection& deselected) {
    button_delete_bind->setEnabled(!selected.isEmpty());
    if (!selected.isEmpty()) {
        const auto dot_data = selected.indexes().first().data(DataRoleDot);
        if (dot_data.isValid()) {
            bottom_screen->HighlightDot(dot_data.toInt());
        }
    }
    if (!deselected.isEmpty()) {
        const auto dot_data = deselected.indexes().first().data(DataRoleDot);
        if (dot_data.isValid()) {
            bottom_screen->HighlightDot(dot_data.toInt(), false);
        }
    }
}

void ConfigureTouchFromButton::OnBindingChanged(QStandardItem* item) {
    if (item->column() == 0) {
        return;
    }

    const bool blocked = binding_list_model->blockSignals(true);
    item->setText(QString::number(
        std::clamp(item->text().toInt(), 0,
                   static_cast<int>((item->column() == 1 ? Layout::ScreenUndocked::Width
                                                         : Layout::ScreenUndocked::Height) -
                                    1))));
    binding_list_model->blockSignals(blocked);

    const auto dot_data = binding_list_model->index(item->row(), 0).data(DataRoleDot);
    if (dot_data.isValid()) {
        bottom_screen->MoveDot(dot_data.toInt(),
                               binding_list_model->item(item->row(), 1)->text().toInt(),
                               binding_list_model->item(item->row(), 2)->text().toInt());
    }
}

void ConfigureTouchFromButton::OnBindingDeleted(const QModelIndex& parent, int first, int last) {
    for (int i = first; i <= last; ++i) {
        const auto ix = binding_list_model->index(i, 0);
        if (!ix.isValid()) {
            return;
        }
        const auto dot_data = ix.data(DataRoleDot);
        if (dot_data.isValid()) {
            bottom_screen->RemoveDot(dot_data.toInt());
        }
    }
}

void ConfigureTouchFromButton::SetActiveBinding(const int dot_id) {
    for (int i = 0; i < binding_list_model->rowCount(); ++i) {
        if (binding_list_model->index(i, 0).data(DataRoleDot) == dot_id) {
            binding_list->setCurrentIndex(binding_list_model->index(i, 0));
            binding_list->setFocus();
            return;
        }
    }
}

void ConfigureTouchFromButton::SetCoordinates(const int dot_id, const QPoint& pos) {
    for (int i = 0; i < binding_list_model->rowCount(); ++i) {
        if (binding_list_model->item(i, 0)->data(DataRoleDot) == dot_id) {
            binding_list_model->item(i, 1)->setText(QString::number(pos.x()));
            binding_list_model->item(i, 2)->setText(QString::number(pos.y()));
            return;
        }
    }
}

void ConfigureTouchFromButton::SetPollingResult(const Common::ParamPackage& params,
                                                const bool cancel) {
    timeout_timer->stop();
    poll_timer->stop();
    input_subsystem->StopMapping();

    releaseKeyboard();
    releaseMouse();
    qApp->restoreOverrideCursor();

    if (input_setter) {
        (*input_setter)(params, cancel);
        input_setter.reset();
    }
}

void ConfigureTouchFromButton::keyPressEvent(QKeyEvent* event) {
    if (!input_setter && event->key() == Qt::Key_Delete) {
        DeleteBinding();
        return;
    }

    if (!input_setter) {
        return QDialog::keyPressEvent(event);
    }

    if (event->key() != Qt::Key_Escape) {
        SetPollingResult(Common::ParamPackage{InputCommon::GenerateKeyboardParam(event->key())},
                         false);
    } else {
        SetPollingResult({}, true);
    }
}

void ConfigureTouchFromButton::ApplyConfiguration() {
    SaveCurrentMapping();
    accept();
}

int ConfigureTouchFromButton::GetSelectedIndex() const {
    return selected_index;
}

std::vector<Settings::TouchFromButtonMap> ConfigureTouchFromButton::GetMaps() const {
    return touch_maps;
}

// TouchScreenPreview implementation remains in configure_touch_from_button.cpp
// since it was originally included in the same translation unit

TouchScreenPreview::TouchScreenPreview(QWidget* parent) : QFrame(parent) {
    setBackgroundRole(QPalette::ColorRole::Base);
}

TouchScreenPreview::~TouchScreenPreview() = default;

void TouchScreenPreview::SetCoordLabel(QLabel* const label) {
    coord_label = label;
}

int TouchScreenPreview::AddDot(const int device_x, const int device_y) {
    QFont dot_font{QStringLiteral("monospace")};
    dot_font.setStyleHint(QFont::Monospace);
    dot_font.setPointSize(20);

    auto* dot = new QLabel(this);
    dot->setAttribute(Qt::WA_TranslucentBackground);
    dot->setFont(dot_font);
    dot->setText(QChar(0xD7)); // U+00D7 Multiplication Sign
    dot->setAlignment(Qt::AlignmentFlag::AlignCenter);
    dot->setProperty(PropId, ++max_dot_id);
    dot->setProperty(PropX, device_x);
    dot->setProperty(PropY, device_y);
    dot->setCursor(Qt::CursorShape::PointingHandCursor);
    dot->setMouseTracking(true);
    dot->installEventFilter(this);
    dot->show();
    PositionDot(dot, device_x, device_y);
    dots.emplace_back(max_dot_id, dot);
    return max_dot_id;
}

void TouchScreenPreview::RemoveDot(const int id) {
    const auto iter = std::find_if(dots.begin(), dots.end(),
                                   [id](const auto& entry) { return entry.first == id; });
    if (iter == dots.cend()) {
        return;
    }

    iter->second->deleteLater();
    dots.erase(iter);
}

void TouchScreenPreview::HighlightDot(const int id, const bool active) const {
    for (const auto& dot : dots) {
        if (dot.first == id) {
            if (dot_highlight_color.isValid()) {
                dot.second->setStyleSheet(
                    active ? QStringLiteral("color: %1").arg(dot_highlight_color.name())
                           : QString{});
            } else {
                dot.second->setForegroundRole(active ? QPalette::ColorRole::LinkVisited
                                                     : QPalette::ColorRole::NoRole);
            }
            if (active) {
                dot.second->raise();
            }
            return;
        }
    }
}

void TouchScreenPreview::MoveDot(const int id, const int device_x, const int device_y) const {
    const auto iter = std::find_if(dots.begin(), dots.end(),
                                   [id](const auto& entry) { return entry.first == id; });
    if (iter == dots.cend()) {
        return;
    }

    iter->second->setProperty(PropX, device_x);
    iter->second->setProperty(PropY, device_y);
    PositionDot(iter->second, device_x, device_y);
}

void TouchScreenPreview::resizeEvent(QResizeEvent* event) {
    if (ignore_resize) {
        return;
    }

    const int target_width = std::min(width(), height() * 4 / 3);
    const int target_height = std::min(height(), width() * 3 / 4);
    if (target_width == width() && target_height == height()) {
        return;
    }
    ignore_resize = true;
    setGeometry((parentWidget()->contentsRect().width() - target_width) / 2, y(), target_width,
                target_height);
    ignore_resize = false;

    if (event->oldSize().width() != target_width || event->oldSize().height() != target_height) {
        for (const auto& dot : dots) {
            PositionDot(dot.second);
        }
    }
}

void TouchScreenPreview::mouseMoveEvent(QMouseEvent* event) {
    if (!coord_label) {
        return;
    }
    const auto pos = MapToDeviceCoords(static_cast<int>(event->position().x()),
                                       static_cast<int>(event->position().y()));
    if (pos) {
        coord_label->setText(QStringLiteral("X: %1, Y: %2").arg(pos->x()).arg(pos->y()));
    } else {
        coord_label->clear();
    }
}

void TouchScreenPreview::leaveEvent(QEvent* event) {
    if (coord_label) {
        coord_label->clear();
    }
}

void TouchScreenPreview::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::MouseButton::LeftButton) {
        return;
    }
    const auto pos = MapToDeviceCoords(static_cast<int>(event->position().x()),
                                       static_cast<int>(event->position().y()));
    if (pos) {
        emit DotAdded(*pos);
    }
}

bool TouchScreenPreview::eventFilter(QObject* obj, QEvent* event) {
    switch (event->type()) {
    case QEvent::Type::MouseButtonPress: {
        const auto mouse_event = static_cast<QMouseEvent*>(event);
        if (mouse_event->button() != Qt::MouseButton::LeftButton) {
            break;
        }
        emit DotSelected(obj->property(PropId).toInt());

        drag_state.dot = qobject_cast<QLabel*>(obj);
        drag_state.start_pos = mouse_event->globalPosition().toPoint();
        return true;
    }
    case QEvent::Type::MouseMove: {
        if (!drag_state.dot) {
            break;
        }
        const auto mouse_event = static_cast<QMouseEvent*>(event);
        if (!drag_state.active) {
            drag_state.active =
                (mouse_event->globalPosition().toPoint() - drag_state.start_pos).manhattanLength() >=
                QApplication::startDragDistance();
            if (!drag_state.active) {
                break;
            }
        }
        auto current_pos = mapFromGlobal(mouse_event->globalPosition().toPoint());
        current_pos.setX(std::clamp(current_pos.x(), contentsMargins().left(),
                                    contentsMargins().left() + contentsRect().width() - 1));
        current_pos.setY(std::clamp(current_pos.y(), contentsMargins().top(),
                                    contentsMargins().top() + contentsRect().height() - 1));
        const auto device_coord = MapToDeviceCoords(current_pos.x(), current_pos.y());
        if (device_coord) {
            drag_state.dot->setProperty(PropX, device_coord->x());
            drag_state.dot->setProperty(PropY, device_coord->y());
            PositionDot(drag_state.dot, device_coord->x(), device_coord->y());
            emit DotMoved(drag_state.dot->property(PropId).toInt(), *device_coord);
            if (coord_label) {
                coord_label->setText(
                    QStringLiteral("X: %1, Y: %2").arg(device_coord->x()).arg(device_coord->y()));
            }
        }
        return true;
    }
    case QEvent::Type::MouseButtonRelease: {
        drag_state.dot.clear();
        drag_state.active = false;
        return true;
    }
    default:
        break;
    }
    return obj->eventFilter(obj, event);
}

std::optional<QPoint> TouchScreenPreview::MapToDeviceCoords(const int screen_x,
                                                            const int screen_y) const {
    const float t_x = 0.5f + static_cast<float>(screen_x - contentsMargins().left()) *
                                 (Layout::ScreenUndocked::Width - 1) / (contentsRect().width() - 1);
    const float t_y = 0.5f + static_cast<float>(screen_y - contentsMargins().top()) *
                                 (Layout::ScreenUndocked::Height - 1) /
                                 (contentsRect().height() - 1);
    if (t_x >= 0.5f && t_x < Layout::ScreenUndocked::Width && t_y >= 0.5f &&
        t_y < Layout::ScreenUndocked::Height) {

        return QPoint{static_cast<int>(t_x), static_cast<int>(t_y)};
    }
    return std::nullopt;
}

void TouchScreenPreview::PositionDot(QLabel* const dot, const int device_x,
                                     const int device_y) const {
    const float device_coord_x =
        static_cast<float>(device_x >= 0 ? device_x : dot->property(PropX).toInt());
    int x_coord = static_cast<int>(
        device_coord_x * (contentsRect().width() - 1) / (Layout::ScreenUndocked::Width - 1) +
        contentsMargins().left() - static_cast<float>(dot->width()) / 2 + 0.5f);

    const float device_coord_y =
        static_cast<float>(device_y >= 0 ? device_y : dot->property(PropY).toInt());
    const int y_coord = static_cast<int>(
        device_coord_y * (contentsRect().height() - 1) / (Layout::ScreenUndocked::Height - 1) +
        contentsMargins().top() - static_cast<float>(dot->height()) / 2 + 0.5f);

    dot->move(x_coord, y_coord);
}
