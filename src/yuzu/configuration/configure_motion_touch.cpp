// SPDX-FileCopyrightText: 2018 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <sstream>

#include <QCloseEvent>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMessageBox>
#include <QPushButton>
#include <QStringListModel>
#include <QVBoxLayout>

#include "common/logging/log.h"
#include "common/settings.h"
#include "input_common/drivers/udp_client.h"
#include "input_common/helpers/udp_protocol.h"
#include "input_common/main.h"
#include "yuzu/configuration/configure_motion_touch.h"
#include "yuzu/configuration/configure_touch_from_button.h"

CalibrationConfigurationDialog::CalibrationConfigurationDialog(QWidget* parent,
                                                               const std::string& host, u16 port)
    : QDialog(parent) {
    layout = new QVBoxLayout;
    status_label = new QLabel(tr("Communicating with the server..."));
    cancel_button = new QPushButton(tr("Cancel"));
    connect(cancel_button, &QPushButton::clicked, this, [this] {
        if (!completed) {
            job->Stop();
        }
        accept();
    });
    layout->addWidget(status_label);
    layout->addWidget(cancel_button);
    setLayout(layout);

    using namespace InputCommon::CemuhookUDP;
    job = std::make_unique<CalibrationConfigurationJob>(
        host, port,
        [this](CalibrationConfigurationJob::Status status) {
            QMetaObject::invokeMethod(this, [status, this] {
                QString text;
                switch (status) {
                case CalibrationConfigurationJob::Status::Ready:
                    text = tr("Touch the top left corner <br>of your touchpad.");
                    break;
                case CalibrationConfigurationJob::Status::Stage1Completed:
                    text = tr("Now touch the bottom right corner <br>of your touchpad.");
                    break;
                case CalibrationConfigurationJob::Status::Completed:
                    text = tr("Configuration completed!");
                    break;
                default:
                    break;
                }
                UpdateLabelText(text);
            });
            if (status == CalibrationConfigurationJob::Status::Completed) {
                QMetaObject::invokeMethod(this, [this] { UpdateButtonText(tr("OK")); });
            }
        },
        [this](u16 min_x_, u16 min_y_, u16 max_x_, u16 max_y_) {
            completed = true;
            min_x = min_x_;
            min_y = min_y_;
            max_x = max_x_;
            max_y = max_y_;
        });
}

CalibrationConfigurationDialog::~CalibrationConfigurationDialog() = default;

void CalibrationConfigurationDialog::UpdateLabelText(const QString& text) {
    status_label->setText(text);
}

void CalibrationConfigurationDialog::UpdateButtonText(const QString& text) {
    cancel_button->setText(text);
}

ConfigureMotionTouch::ConfigureMotionTouch(QWidget* parent,
                                           InputCommon::InputSubsystem* input_subsystem_)
    : QDialog(parent), input_subsystem{input_subsystem_} {
    setWindowTitle(tr("Configure Motion / Touch"));

    auto* main_layout = new QVBoxLayout(this);

    // Touch group
    auto* touch_group = new QGroupBox(tr("Touch"), this);
    auto* touch_layout = new QVBoxLayout(touch_group);

    // Calibration row
    auto* calib_layout = new QHBoxLayout;
    touch_calibration_label = new QLabel(tr("UDP Calibration:"), this);
    calib_layout->addWidget(touch_calibration_label);

    touch_calibration = new QLabel(QStringLiteral("(100, 50) - (1800, 850)"), this);
    touch_calibration->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    calib_layout->addWidget(touch_calibration);

    touch_calibration_config = new QPushButton(tr("Configure"), this);
    touch_calibration_config->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    calib_layout->addWidget(touch_calibration_config);

    touch_layout->addLayout(calib_layout);

    // Touch from button row
    auto* tfb_layout = new QHBoxLayout;
    tfb_layout->addWidget(new QLabel(tr("Touch from button profile:"), this));

    touch_from_button_map = new QComboBox(this);
    tfb_layout->addWidget(touch_from_button_map);

    touch_from_button_config_btn = new QPushButton(tr("Configure"), this);
    touch_from_button_config_btn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    tfb_layout->addWidget(touch_from_button_config_btn);

    touch_layout->addLayout(tfb_layout);
    main_layout->addWidget(touch_group);

    // CemuhookUDP Config group
    auto* udp_group = new QGroupBox(tr("CemuhookUDP Config"), this);
    auto* udp_layout = new QVBoxLayout(udp_group);

    auto* udp_help = new QLabel(
        tr("You may use any Cemuhook compatible UDP input source to provide motion and touch "
           "input."),
        this);
    udp_help->setWordWrap(true);
    udp_layout->addWidget(udp_help);

    auto* udp_content_layout = new QHBoxLayout;

    // Server list
    udp_server_list = new QListView(this);
    udp_content_layout->addWidget(udp_server_list);

    // Right side controls
    auto* udp_controls_layout = new QVBoxLayout;
    udp_controls_layout->setContentsMargins(0, 0, 0, 0);

    // Server input
    auto* server_layout = new QHBoxLayout;
    server_layout->setContentsMargins(3, 3, 0, 0);
    server_layout->addWidget(new QLabel(tr("Server:"), this));
    udp_server = new QLineEdit(this);
    udp_server->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    server_layout->addWidget(udp_server);
    udp_controls_layout->addLayout(server_layout);

    // Port input
    auto* port_layout = new QHBoxLayout;
    port_layout->setContentsMargins(3, 0, 0, 0);
    port_layout->addWidget(new QLabel(tr("Port:"), this));
    udp_port = new QLineEdit(this);
    udp_port->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    port_layout->addWidget(udp_port);
    udp_controls_layout->addLayout(port_layout);

    // Learn more + Test + Add
    auto* actions_layout = new QHBoxLayout;
    actions_layout->setContentsMargins(3, 0, 0, 0);

    udp_learn_more = new QLabel(this);
    udp_learn_more->setOpenExternalLinks(true);
    udp_learn_more->setText(
        tr("<a "
           "href='https://yuzu-emu.org/wiki/"
           "using-a-controller-or-android-phone-for-motion-or-touch-input'><span "
           "style=\"text-decoration: underline; color:#039be5;\">Learn More</span></a>"));
    actions_layout->addWidget(udp_learn_more);

    udp_test = new QPushButton(tr("Test"), this);
    udp_test->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    actions_layout->addWidget(udp_test);

    udp_add = new QPushButton(tr("Add Server"), this);
    udp_add->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    actions_layout->addWidget(udp_add);

    udp_controls_layout->addLayout(actions_layout);

    udp_controls_layout->addStretch();

    // Remove button
    auto* remove_layout = new QHBoxLayout;
    udp_remove = new QPushButton(tr("Remove Server"), this);
    udp_remove->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    remove_layout->addWidget(udp_remove);
    remove_layout->addStretch();

    udp_controls_layout->addLayout(remove_layout);

    udp_content_layout->addLayout(udp_controls_layout);
    udp_layout->addLayout(udp_content_layout);

    main_layout->addWidget(udp_group);
    main_layout->addStretch();

    // Button box
    button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    main_layout->addWidget(button_box);

    SetConfiguration();
    UpdateUiDisplay();
    ConnectEvents();
}

ConfigureMotionTouch::~ConfigureMotionTouch() = default;

void ConfigureMotionTouch::SetConfiguration() {
    const Common::ParamPackage touch_param(Settings::values.touch_device.GetValue());

    touch_from_button_maps = Settings::values.touch_from_button_maps;
    for (const auto& touch_map : touch_from_button_maps) {
        touch_from_button_map->addItem(QString::fromStdString(touch_map.name));
    }
    touch_from_button_map->setCurrentIndex(
        Settings::values.touch_from_button_map_index.GetValue());

    min_x = touch_param.Get("min_x", 100);
    min_y = touch_param.Get("min_y", 50);
    max_x = touch_param.Get("max_x", 1800);
    max_y = touch_param.Get("max_y", 850);

    udp_server->setText(QString::fromStdString("127.0.0.1"));
    udp_port->setText(QString::number(26760));

    udp_server_list_model = new QStringListModel(this);
    udp_server_list_model->setStringList({});
    udp_server_list->setModel(udp_server_list_model);

    std::stringstream ss(Settings::values.udp_input_servers.GetValue());
    std::string token;

    while (std::getline(ss, token, ',')) {
        const int row = udp_server_list_model->rowCount();
        udp_server_list_model->insertRows(row, 1);
        const QModelIndex index = udp_server_list_model->index(row);
        udp_server_list_model->setData(index, QString::fromStdString(token));
    }
}

void ConfigureMotionTouch::UpdateUiDisplay() {
    touch_calibration->setText(
        QStringLiteral("(%1, %2) - (%3, %4)").arg(min_x).arg(min_y).arg(max_x).arg(max_y));
}

void ConfigureMotionTouch::ConnectEvents() {
    connect(udp_test, &QPushButton::clicked, this, &ConfigureMotionTouch::OnCemuhookUDPTest);
    connect(udp_add, &QPushButton::clicked, this, &ConfigureMotionTouch::OnUDPAddServer);
    connect(udp_remove, &QPushButton::clicked, this, &ConfigureMotionTouch::OnUDPDeleteServer);
    connect(touch_calibration_config, &QPushButton::clicked, this,
            &ConfigureMotionTouch::OnConfigureTouchCalibration);
    connect(touch_from_button_config_btn, &QPushButton::clicked, this,
            &ConfigureMotionTouch::OnConfigureTouchFromButton);
    connect(button_box, &QDialogButtonBox::accepted, this,
            &ConfigureMotionTouch::ApplyConfiguration);
    connect(button_box, &QDialogButtonBox::rejected, this, [this] {
        if (CanCloseDialog()) {
            reject();
        }
    });
}

void ConfigureMotionTouch::OnUDPAddServer() {
    const QRegularExpression re(QStringLiteral(
        R"re(^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)re"));
    bool ok;
    const QString port_text = udp_port->text();
    const QString server_text = udp_server->text();
    const QString server_string = tr("%1:%2").arg(server_text, port_text);
    const int port_number = port_text.toInt(&ok, 10);
    const int row = udp_server_list_model->rowCount();

    if (!ok) {
        QMessageBox::warning(this, tr("yuzu"), tr("Port number has invalid characters"));
        return;
    }
    if (port_number < 0 || port_number > 65353) {
        QMessageBox::warning(this, tr("yuzu"), tr("Port has to be in range 0 and 65353"));
        return;
    }
    if (!re.match(server_text).hasMatch()) {
        QMessageBox::warning(this, tr("yuzu"), tr("IP address is not valid"));
        return;
    }
    for (const auto& item : udp_server_list_model->stringList()) {
        if (item == server_string) {
            QMessageBox::warning(this, tr("yuzu"), tr("This UDP server already exists"));
            return;
        }
    }
    if (row == 8) {
        QMessageBox::warning(this, tr("yuzu"), tr("Unable to add more than 8 servers"));
        return;
    }

    udp_server_list_model->insertRows(row, 1);
    QModelIndex index = udp_server_list_model->index(row);
    udp_server_list_model->setData(index, server_string);
    udp_server_list->setCurrentIndex(index);
}

void ConfigureMotionTouch::OnUDPDeleteServer() {
    udp_server_list_model->removeRows(udp_server_list->currentIndex().row(), 1);
}

void ConfigureMotionTouch::OnCemuhookUDPTest() {
    udp_test->setEnabled(false);
    udp_test->setText(tr("Testing"));
    udp_test_in_progress = true;
    InputCommon::CemuhookUDP::TestCommunication(
        udp_server->text().toStdString(), static_cast<u16>(udp_port->text().toInt()),
        [this] {
            LOG_INFO(Frontend, "UDP input test success");
            QMetaObject::invokeMethod(this, [this] { ShowUDPTestResult(true); });
        },
        [this] {
            LOG_ERROR(Frontend, "UDP input test failed");
            QMetaObject::invokeMethod(this, [this] { ShowUDPTestResult(false); });
        });
}

void ConfigureMotionTouch::OnConfigureTouchCalibration() {
    touch_calibration_config->setEnabled(false);
    touch_calibration_config->setText(tr("Configuring"));
    CalibrationConfigurationDialog dialog(this, udp_server->text().toStdString(),
                                          static_cast<u16>(udp_port->text().toUInt()));
    dialog.exec();
    if (dialog.completed) {
        min_x = dialog.min_x;
        min_y = dialog.min_y;
        max_x = dialog.max_x;
        max_y = dialog.max_y;
        LOG_INFO(Frontend,
                 "UDP touchpad calibration config success: min_x={}, min_y={}, max_x={}, max_y={}",
                 min_x, min_y, max_x, max_y);
        UpdateUiDisplay();
    } else {
        LOG_ERROR(Frontend, "UDP touchpad calibration config failed");
    }
    touch_calibration_config->setEnabled(true);
    touch_calibration_config->setText(tr("Configure"));
}

void ConfigureMotionTouch::closeEvent(QCloseEvent* event) {
    if (CanCloseDialog()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void ConfigureMotionTouch::ShowUDPTestResult(bool result) {
    udp_test_in_progress = false;
    if (result) {
        QMessageBox::information(this, tr("Test Successful"),
                                 tr("Successfully received data from the server."));
    } else {
        QMessageBox::warning(this, tr("Test Failed"),
                             tr("Could not receive valid data from the server.<br>Please verify "
                                "that the server is set up correctly and "
                                "the address and port are correct."));
    }
    udp_test->setEnabled(true);
    udp_test->setText(tr("Test"));
}

void ConfigureMotionTouch::OnConfigureTouchFromButton() {
    ConfigureTouchFromButton dialog{this, touch_from_button_maps, input_subsystem,
                                    touch_from_button_map->currentIndex()};
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    touch_from_button_maps = dialog.GetMaps();

    while (touch_from_button_map->count() > 0) {
        touch_from_button_map->removeItem(0);
    }
    for (const auto& touch_map : touch_from_button_maps) {
        touch_from_button_map->addItem(QString::fromStdString(touch_map.name));
    }
    touch_from_button_map->setCurrentIndex(dialog.GetSelectedIndex());
}

bool ConfigureMotionTouch::CanCloseDialog() {
    if (udp_test_in_progress) {
        QMessageBox::warning(this, tr("yuzu"),
                             tr("UDP Test or calibration configuration is in progress.<br>Please "
                                "wait for them to finish."));
        return false;
    }
    return true;
}

void ConfigureMotionTouch::ApplyConfiguration() {
    if (!CanCloseDialog()) {
        return;
    }

    Common::ParamPackage touch_param{};
    touch_param.Set("min_x", min_x);
    touch_param.Set("min_y", min_y);
    touch_param.Set("max_x", max_x);
    touch_param.Set("max_y", max_y);

    Settings::values.touch_device = touch_param.Serialize();
    Settings::values.touch_from_button_map_index = touch_from_button_map->currentIndex();
    Settings::values.touch_from_button_maps = touch_from_button_maps;
    Settings::values.udp_input_servers = GetUDPServerString();
    input_subsystem->ReloadInputDevices();

    accept();
}

std::string ConfigureMotionTouch::GetUDPServerString() const {
    QString input_servers;

    for (const auto& item : udp_server_list_model->stringList()) {
        input_servers += item;
        input_servers += QLatin1Char{','};
    }

    // Remove last comma
    input_servers.chop(1);
    return input_servers.toStdString();
}
