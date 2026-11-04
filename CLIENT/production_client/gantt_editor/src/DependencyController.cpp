#include "DependencyController.h"

DependencyController::DependencyController(ProjectData* projectData, SettingsManager* settingsManager, QObject *parent)
    : QObject(parent), m_projectData(projectData), m_settingsManager(settingsManager) {}

void DependencyController::addDependency(const QString& predecessorId, const QString& successorId)
{
    if (m_settingsManager->editingLocked()) {
        emit errorOccurred("Редактирование заблокировано");
        return;
    }
    QVariantMap predTask = m_projectData->get_taskModel()->getTask(predecessorId);
    QVariantMap succTask = m_projectData->get_taskModel()->getTask(successorId);
    if (predTask.isEmpty() || succTask.isEmpty()) {
        emit errorOccurred("Задача не найдена");
        return;
    }

    bool predHasDates = predTask["startDate"].toDate().isValid() && predTask["endDate"].toDate().isValid();
    bool succHasDates = succTask["startDate"].toDate().isValid() && succTask["endDate"].toDate().isValid();

    if (!predHasDates && succHasDates) {
        emit errorOccurred("Задача без сроков не может быть предшественником задачи со сроками");
        return;
    }

    if (!m_projectData->get_dependencyModel()->addDependency(predecessorId, successorId)) {
        emit errorOccurred("Невозможно создать зависимость (циклическая или уже существует)");
    } else {
        m_projectData->set_Modified(true);
    }
}

void DependencyController::removeDependency(const QString& dependencyId)
{
    if (m_settingsManager->editingLocked()) {
        emit errorOccurred("Редактирование заблокировано");
        return;
    }
    m_projectData->get_dependencyModel()->removeDependency(dependencyId);
    m_projectData->set_Modified(true);
}
