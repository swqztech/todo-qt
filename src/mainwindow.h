#pragma once

#include "todostore.h"

#include <QListWidgetItem>
#include <QMainWindow>

class QLineEdit;
class QListWidget;
class QPushButton;
class QLabel;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = nullptr);

 private slots:
  void onAdd();
  void onFilterAll();
  void onFilterActive();
  void onFilterDone();
  void onExport();
  void onImport();
  void onPaste();
  void onMoveUp();
  void onMoveDown();
  void onDelete();
  void onItemChanged(QListWidgetItem* item);
  void onEditItem(QListWidgetItem* item);

 private:
  void refreshList();
  void styleChip(QPushButton* btn, bool on);
  int selectedStoreIndex() const;

  TodoStore store_;
  FilterKind filter_ = FilterKind::All;
  bool suppress_ = false;

  QLineEdit* input_ = nullptr;
  QListWidget* list_ = nullptr;
  QLabel* stats_ = nullptr;
  QLabel* date_ = nullptr;
  QPushButton* btn_all_ = nullptr;
  QPushButton* btn_active_ = nullptr;
  QPushButton* btn_done_ = nullptr;
};
