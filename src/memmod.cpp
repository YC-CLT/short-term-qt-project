#include "memmod.h"

MemMod::MemMod(QWidget* parent, Ui::MainWindow* mainUi):QWidget(parent), ui(mainUi)
{
    if(!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        QMessageBox::critical(this, "错误", "SQLite驱动未加载");
        return;
    }
    memDb = QSqlDatabase::addDatabase("QSQLITE", "memmod_connection");  // Add unique connection name
    
    // 自动搜索路径列表
    QStringList dbPaths = {
        "mem.db",  // 当前目录
        "../mem.db",  // 上级目录
        "data/mem.db"  // 下级data目录
    };
    
    bool dbOpened = false;
    for(const auto& path : dbPaths) {
        memDb.setDatabaseName(path);
        if(memDb.open()) {
            dbOpened = true;
            break;
        }
    }
    
    if(!dbOpened) {
        // 让用户选择数据库文件位置
        QString fileName = QFileDialog::getSaveFileName(this, 
            tr("选择或创建数据库文件"), 
            QDir::currentPath(), 
            tr("SQLite数据库文件 (*.db)"));
        
        if(fileName.isEmpty()) {
            QMessageBox::warning(this, "错误", "必须选择数据库文件位置");
            return;
        }
        
        memDb.setDatabaseName(fileName);
        if(!memDb.open()) {
            QMessageBox::warning(this, "数据库打开失败", memDb.lastError().text());
            return;
        }
    }
    // 检查表是否存在
    if(!memDb.tables().contains("personal_mem")) {
        QSqlQuery query(memDb);
        
        // 创建主表
        QString createTable = "CREATE TABLE personal_mem ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "mem_content TEXT NOT NULL, "
            "created_at DATETIME DEFAULT (datetime('now','localtime')), "
            "updated_at DATETIME DEFAULT (datetime('now','localtime')))";
        
        if(!query.exec(createTable)) {
            QMessageBox::warning(this, "创建表失败", query.lastError().text());
            return;
        }

        // 创建触发器和日志表
        QStringList triggers = {
            "CREATE TRIGGER update_mem_time AFTER UPDATE ON personal_mem "
            "BEGIN UPDATE personal_mem SET updated_at = (datetime('now','localtime')) WHERE id = OLD.id; END",
            
            "CREATE TABLE mem_log ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "mem_id INTEGER NOT NULL, "
            "operation TEXT NOT NULL, "
            "operated_at DATETIME DEFAULT (datetime('now','localtime')))",
            
            "CREATE TRIGGER log_insert_mem AFTER INSERT ON personal_mem "
            "BEGIN INSERT INTO mem_log (mem_id, operation) VALUES (NEW.id, '新增 id:'||NEW.id||', 内容:'||NEW.mem_content); END",
            
            "CREATE TRIGGER log_update_mem AFTER UPDATE ON personal_mem "
            "BEGIN INSERT INTO mem_log (mem_id, operation) VALUES (OLD.id, '更改 id:'||OLD.id||', 内容: '||OLD.mem_content||' -> '||NEW.mem_content); END",
            
            "CREATE TRIGGER log_delete_mem BEFORE DELETE ON personal_mem "
            "BEGIN INSERT INTO mem_log (mem_id, operation) VALUES (OLD.id, '删除 id:'||OLD.id||', 内容:'||OLD.mem_content); END"
        };

        for(const auto& trigger : triggers) {
            if(!query.exec(trigger)) {
                QMessageBox::warning(this, "创建触发器失败", query.lastError().text());
                return;
            }
        }        
    }

// 初始化数据模型
    model = new QSqlTableModel(this, memDb);
    model->setTable("personal_mem");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

    // 设置表头
    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("备忘录内容"));
    model->setHeaderData(2, Qt::Horizontal, tr("创建时间"));
    model->setHeaderData(3, Qt::Horizontal, tr("更新时间"));

    QHeaderView* header = ui->memTableView->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Interactive);
    header->setDefaultAlignment(Qt::AlignCenter);  // 设置表头居中
    
    ui->memTableView->setModel(model);

    header->setSectionResizeMode(QHeaderView::Interactive);
    header->resizeSection(0, 60);   // ID列
    header->resizeSection(1, 400); // 内容列
    header->resizeSection(2, 150);  // 创建时间
    header->resizeSection(3, 150); // 更新时间

    ui->memTableView->setItemDelegate(new CenterAlignDelegate(this));
}

// 实现其他方法（与重要日模块类似）
void MemMod::refreshMemTable() {
    model->select();
}

void MemMod::commitMemTable() {
    if(!model) return;
    
    memDb.transaction();
    if(!model->submitAll()) {
        memDb.rollback();
        QMessageBox::warning(this, "提交失败", model->lastError().text());
        return;
    }
    
    if(!memDb.commit()) {
        QMessageBox::warning(this, "提交失败", memDb.lastError().text());
        return;
    }
    
    QMessageBox::information(this, "成功", "数据已提交");
    refreshMemTable();
}

void MemMod::deleteMemTable() {
    if(!model) return;
    
    QModelIndexList selectedRows = ui->memTableView->selectionModel()->selectedRows();
    if(selectedRows.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要删除的行");
        return;
    }
    
    if(QMessageBox::question(this, "确认删除", "确定删除选中的行吗？") != QMessageBox::Yes) {
        return;
    }
    
    memDb.transaction();
    for(const auto& index : selectedRows) {
        model->removeRow(index.row());
    }
    
    if(!model->submitAll()) {
        memDb.rollback();
        QMessageBox::warning(this, "删除失败", model->lastError().text());
        return;
    }
    
    if(!memDb.commit()) {
        QMessageBox::warning(this, "删除失败", memDb.lastError().text());
        return;
    }
}

void MemMod::cancelMemTable() {
    if(!model) return;
    model->revertAll();
}

void MemMod::insertMemTable() {
    if(!model) return;
    
    int row = model->rowCount();
    if(!model->insertRow(row)) {
        QMessageBox::warning(this, "错误", "无法插入新行");
        return;
    }
    
    QModelIndex contentIndex = model->index(row, 1);
    model->setData(contentIndex, tr("新备忘录"));
    
    ui->memTableView->scrollToBottom();
    ui->memTableView->setCurrentIndex(contentIndex);
    ui->memTableView->viewport()->update();
}

MemMod::~MemMod()
{
    if(memDb.isOpen()) {
        QString connectionName = memDb.connectionName();
        memDb.close();
        QSqlDatabase::removeDatabase(connectionName);
    }
}
