CREATE DATABASE IF NOT EXISTS shop
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

USE shop;

CREATE TABLE IF NOT EXISTS `User` (
    user_id INT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(50) NOT NULL UNIQUE,
    email VARCHAR(120) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    phone VARCHAR(30) DEFAULT '',
    status VARCHAR(20) NOT NULL DEFAULT '正常',
    preference_category VARCHAR(50) NOT NULL DEFAULT '电子数码',
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `Admin` (
    admin_id INT PRIMARY KEY AUTO_INCREMENT,
    admin_name VARCHAR(50) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    phone VARCHAR(30) DEFAULT '',
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `Category` (
    category_id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(50) NOT NULL UNIQUE,
    description VARCHAR(255) DEFAULT ''
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `Goods` (
    goods_id INT PRIMARY KEY AUTO_INCREMENT,
    user_id INT NOT NULL,
    category_id INT NOT NULL,
    title VARCHAR(120) NOT NULL,
    description TEXT,
    price DECIMAL(10, 2) NOT NULL,
    stock INT NOT NULL DEFAULT 1,
    condition_level INT NOT NULL DEFAULT 8,
    image_path VARCHAR(500) DEFAULT '',
    status VARCHAR(20) NOT NULL DEFAULT '待审核',
    sales_count INT NOT NULL DEFAULT 0,
    is_deleted TINYINT(1) NOT NULL DEFAULT 0,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    CONSTRAINT fk_goods_user FOREIGN KEY (user_id) REFERENCES `User`(user_id),
    CONSTRAINT fk_goods_category FOREIGN KEY (category_id) REFERENCES `Category`(category_id),
    CONSTRAINT chk_goods_price CHECK (price >= 0),
    CONSTRAINT chk_goods_stock CHECK (stock >= 0),
    CONSTRAINT chk_goods_condition CHECK (condition_level BETWEEN 1 AND 10)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `Orders` (
    order_id INT PRIMARY KEY AUTO_INCREMENT,
    user_id INT NOT NULL,
    goods_id INT NOT NULL,
    quantity INT NOT NULL DEFAULT 1,
    total_price DECIMAL(10, 2) NOT NULL,
    status VARCHAR(20) NOT NULL DEFAULT '未完成',
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    CONSTRAINT fk_orders_user FOREIGN KEY (user_id) REFERENCES `User`(user_id),
    CONSTRAINT fk_orders_goods FOREIGN KEY (goods_id) REFERENCES `Goods`(goods_id),
    CONSTRAINT chk_orders_quantity CHECK (quantity > 0),
    CONSTRAINT chk_orders_total CHECK (total_price >= 0)
) ENGINE=InnoDB;

CREATE OR REPLACE VIEW view_goods_detail AS
SELECT
    g.goods_id,
    g.title,
    c.name AS category_name,
    g.price,
    g.stock,
    g.sales_count,
    g.status,
    u.username AS seller,
    g.description,
    g.condition_level,
    g.image_path,
    g.created_at
FROM `Goods` g
JOIN `Category` c ON c.category_id = g.category_id
JOIN `User` u ON u.user_id = g.user_id
WHERE g.is_deleted = 0;

INSERT INTO `User` (username, email, password_hash, phone, status, preference_category)
VALUES ('demo_user', 'demo_user@example.com', 'demo123', '07123456789', '正常', '电子数码')
ON DUPLICATE KEY UPDATE username = VALUES(username);

INSERT INTO `Admin` (admin_name, password_hash, phone)
VALUES ('admin', 'admin123', '07000000000')
ON DUPLICATE KEY UPDATE admin_name = VALUES(admin_name);

INSERT INTO `Category` (name, description) VALUES
    ('书籍教材', 'Textbooks and study materials'),
    ('电子数码', 'Phones, computers and accessories'),
    ('生活用品', 'Everyday dormitory essentials'),
    ('服饰鞋包', 'Clothing, shoes and bags'),
    ('运动器材', 'Sports and fitness equipment'),
    ('代步工具', 'Bikes and campus transport')
ON DUPLICATE KEY UPDATE description = VALUES(description);

INSERT INTO `Goods` (user_id, category_id, title, description, price, stock, condition_level, status, sales_count)
SELECT u.user_id, c.category_id, 'C++ 程序设计', 'A well-kept programming textbook.', 25.00, 6, 8, '已上架', 16
FROM `User` u JOIN `Category` c ON c.name = '书籍教材'
WHERE u.username = 'demo_user'
  AND NOT EXISTS (SELECT 1 FROM `Goods` WHERE title = 'C++ 程序设计');

INSERT INTO `Goods` (user_id, category_id, title, description, price, stock, condition_level, status, sales_count)
SELECT u.user_id, c.category_id, '蓝牙耳机', 'Wireless headphones in good working condition.', 99.00, 2, 8, '已上架', 19
FROM `User` u JOIN `Category` c ON c.name = '电子数码'
WHERE u.username = 'demo_user'
  AND NOT EXISTS (SELECT 1 FROM `Goods` WHERE title = '蓝牙耳机');

INSERT INTO `Goods` (user_id, category_id, title, description, price, stock, condition_level, status, sales_count)
SELECT u.user_id, c.category_id, '羽毛球拍', 'Lightweight racket suitable for campus sports.', 88.00, 4, 9, '已上架', 13
FROM `User` u JOIN `Category` c ON c.name = '运动器材'
WHERE u.username = 'demo_user'
  AND NOT EXISTS (SELECT 1 FROM `Goods` WHERE title = '羽毛球拍');
