#include "mainwindow.h"

#include <QApplication>
#include <QDate>
#include <QFile>
#include <QClipboard>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
  setWindowTitle(QStringLiteral("墨迹清单"));
  resize(560, 640);
  setStyleSheet(QStringLiteral(
      "QMainWindow { background: #E8EEF6; }"
      "QLabel { color: #1E2834; }"
      "QLineEdit { background: white; border: 1px solid #D2DAE6; padding: 8px; }"
      "QListWidget { background: white; border: 1px solid #D2DAE6; }"
      "QPushButton { background: #F8FAFD; border: 1px solid #D2DAE6; padding: 6px 10px; }"));

  auto* central = new QWidget(this);
  setCentralWidget(central);
  auto* layout = new QVBoxLayout(central);
  layout->setContentsMargins(20, 12, 20, 16);

  auto* top = new QLabel(QStringLiteral("墨迹清单"));
  top->setStyleSheet("font-weight: 600; background: #F5F8FC; padding: 12px;");
  layout->addWidget(top);

  date_ = new QLabel;
  date_->setStyleSheet("font-size: 20px; font-weight: bold; color: #4878C8;");
  const auto now = QDate::currentDate();
  static const char* wk[] = {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};
  const int dow = now.dayOfWeek();
  const int wi = (dow == 7) ? 0 : dow;
  date_->setText(QStringLiteral("%1月%2日%3")
                     .arg(now.month())
                     .arg(now.day())
                     .arg(QString::fromUtf8(wk[wi])));
  layout->addWidget(new QLabel(QStringLiteral("INK LIST")));
  layout->addWidget(date_);
  layout->addWidget(new QLabel(QStringLiteral("写下来，划掉，继续。支持导出 / 导入 / 粘贴 Markdown。")));

  auto* in_row = new QHBoxLayout;
  input_ = new QLineEdit;
  input_->setPlaceholderText(QStringLiteral("下一件要做的事…"));
  connect(input_, &QLineEdit::returnPressed, this, &MainWindow::onAdd);
  in_row->addWidget(input_, 1);
  auto* add_btn = new QPushButton(QStringLiteral("加入"));
  add_btn->setStyleSheet("background: #4882DC; color: white; font-weight: 600;");
  connect(add_btn, &QPushButton::clicked, this, &MainWindow::onAdd);
  in_row->addWidget(add_btn);
  layout->addLayout(in_row);

  auto* chip_row = new QHBoxLayout;
  btn_all_ = new QPushButton(QStringLiteral("全部"));
  btn_active_ = new QPushButton(QStringLiteral("未完成"));
  btn_done_ = new QPushButton(QStringLiteral("已完成"));
  connect(btn_all_, &QPushButton::clicked, this, &MainWindow::onFilterAll);
  connect(btn_active_, &QPushButton::clicked, this, &MainWindow::onFilterActive);
  connect(btn_done_, &QPushButton::clicked, this, &MainWindow::onFilterDone);
  chip_row->addWidget(btn_all_);
  chip_row->addWidget(btn_active_);
  chip_row->addWidget(btn_done_);
  auto* ex = new QPushButton(QStringLiteral("导出"));
  auto* im = new QPushButton(QStringLiteral("导入"));
  auto* pa = new QPushButton(QStringLiteral("粘贴导入"));
  connect(ex, &QPushButton::clicked, this, &MainWindow::onExport);
  connect(im, &QPushButton::clicked, this, &MainWindow::onImport);
  connect(pa, &QPushButton::clicked, this, &MainWindow::onPaste);
  chip_row->addWidget(ex);
  chip_row->addWidget(im);
  chip_row->addWidget(pa);
  stats_ = new QLabel;
  stats_->setStyleSheet("color: #788291;");
  chip_row->addStretch();
  chip_row->addWidget(stats_);
  layout->addLayout(chip_row);

  list_ = new QListWidget;
  list_->setMinimumHeight(280);
  connect(list_, &QListWidget::itemChanged, this, &MainWindow::onItemChanged);
  connect(list_, &QListWidget::itemDoubleClicked, this, &MainWindow::onEditItem);
  layout->addWidget(list_, 1);

  auto* act_row = new QHBoxLayout;
  auto* up = new QPushButton(QStringLiteral("上移"));
  auto* down = new QPushButton(QStringLiteral("下移"));
  auto* del = new QPushButton(QStringLiteral("删除"));
  connect(up, &QPushButton::clicked, this, &MainWindow::onMoveUp);
  connect(down, &QPushButton::clicked, this, &MainWindow::onMoveDown);
  connect(del, &QPushButton::clicked, this, &MainWindow::onDelete);
  act_row->addWidget(up);
  act_row->addWidget(down);
  act_row->addWidget(del);
  layout->addLayout(act_row);

  store_.load();
  refreshList();
}

void MainWindow::styleChip(QPushButton* btn, bool on) {
  if (on)
    btn->setStyleSheet("background: #28303A; color: white; border: 1px solid #4878C8; font-weight: 600;");
  else
    btn->setStyleSheet("background: #F8FAFD; color: #1E2834; border: 1px solid #D2DAE6;");
}

void MainWindow::refreshList() {
  suppress_ = true;
  list_->clear();
  for (int idx : store_.filteredIndices(filter_)) {
    const auto& it = store_.items()[idx];
    auto* item = new QListWidgetItem(it.text);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    item->setCheckState(it.done ? Qt::Checked : Qt::Unchecked);
    item->setData(Qt::UserRole, idx);
    list_->addItem(item);
  }
  suppress_ = false;
  stats_->setText(store_.countsLabel());
  styleChip(btn_all_, filter_ == FilterKind::All);
  styleChip(btn_active_, filter_ == FilterKind::Active);
  styleChip(btn_done_, filter_ == FilterKind::Done);
}

void MainWindow::onAdd() {
  store_.add(input_->text());
  input_->clear();
  refreshList();
}

void MainWindow::onFilterAll() {
  filter_ = FilterKind::All;
  refreshList();
}
void MainWindow::onFilterActive() {
  filter_ = FilterKind::Active;
  refreshList();
}
void MainWindow::onFilterDone() {
  filter_ = FilterKind::Done;
  refreshList();
}

int MainWindow::selectedStoreIndex() const {
  const auto* item = list_->currentItem();
  if (!item) return -1;
  return item->data(Qt::UserRole).toInt();
}

void MainWindow::onItemChanged(QListWidgetItem* item) {
  if (suppress_ || !item) return;
  const int idx = item->data(Qt::UserRole).toInt();
  store_.setDone(idx, item->checkState() == Qt::Checked);
  refreshList();
}

void MainWindow::onEditItem(QListWidgetItem* item) {
  if (!item) return;
  const int idx = item->data(Qt::UserRole).toInt();
  const auto text = QInputDialog::getText(this, QStringLiteral("编辑待办"), QStringLiteral("内容"),
                                          QLineEdit::Normal, item->text());
  if (text.trimmed().isEmpty()) return;
  store_.updateText(idx, text);
  refreshList();
}

void MainWindow::onDelete() {
  const int idx = selectedStoreIndex();
  if (idx < 0) {
    QMessageBox::information(this, QStringLiteral("墨迹清单"), QStringLiteral("请先选中一条待办"));
    return;
  }
  if (QMessageBox::question(this, QStringLiteral("墨迹清单"), QStringLiteral("删除这条待办？")) !=
      QMessageBox::Yes)
    return;
  store_.removeAt(idx);
  refreshList();
}

void MainWindow::onMoveUp() {
  const int idx = selectedStoreIndex();
  if (idx < 0) return;
  store_.moveItem(idx, idx - 1);
  refreshList();
}

void MainWindow::onMoveDown() {
  const int idx = selectedStoreIndex();
  if (idx < 0) return;
  store_.moveItem(idx, idx + 1);
  refreshList();
}

void MainWindow::onExport() {
  const QString path = QFileDialog::getSaveFileName(this, QStringLiteral("导出"),
                                                    QStringLiteral("墨迹清单-export.md"),
                                                    QStringLiteral("Markdown (*.md *.txt)"));
  if (path.isEmpty()) return;
  QFile f(path);
  if (!f.open(QIODevice::WriteOnly)) return;
  f.write(store_.exportMarkdown().toUtf8());
  QMessageBox::information(this, QStringLiteral("墨迹清单"), QStringLiteral("已导出"));
}

void MainWindow::onImport() {
  const QString path = QFileDialog::getOpenFileName(
      this, QStringLiteral("导入"), {}, QStringLiteral("文档 (*.md *.txt *.json)"));
  if (path.isEmpty()) return;
  if (path.endsWith(QStringLiteral(".json"), Qt::CaseInsensitive))
    store_.replaceFromJsonFile(path);
  else {
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) store_.importMarkdown(QString::fromUtf8(f.readAll()));
  }
  refreshList();
  QMessageBox::information(this, QStringLiteral("墨迹清单"), QStringLiteral("导入完成"));
}

void MainWindow::onPaste() {
  const auto* clip = QApplication::clipboard();
  const auto text = clip ? clip->text() : QString();
  if (text.trimmed().isEmpty()) {
    QMessageBox::information(this, QStringLiteral("墨迹清单"), QStringLiteral("剪贴板为空"));
    return;
  }
  store_.importMarkdown(text);
  refreshList();
  QMessageBox::information(this, QStringLiteral("墨迹清单"), QStringLiteral("已从剪贴板导入"));
}
