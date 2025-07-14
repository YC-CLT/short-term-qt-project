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
        qDebug() << "未找到数据库文件";
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
            QMessageBox::information(this, "初始化成功", "数据库初始化成功");
        }
        QMessageBox::information(this, "加载成功","数据库加载成功");


        
    }
}
MemMod::~MemMod()
{
    if(memDb.isOpen()) {
        QString connectionName = memDb.connectionName();
        memDb.close();
        QSqlDatabase::removeDatabase(connectionName);
    }
}
