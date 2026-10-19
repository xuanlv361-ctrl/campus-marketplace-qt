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
    INDEX idx_goods_category_status (category_id, status),
    INDEX idx_goods_seller (user_id),
    INDEX idx_goods_title (title),
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
    INDEX idx_orders_user_status (user_id, status),
    INDEX idx_orders_goods (goods_id),
    CONSTRAINT fk_orders_user FOREIGN KEY (user_id) REFERENCES `User`(user_id),
    CONSTRAINT fk_orders_goods FOREIGN KEY (goods_id) REFERENCES `Goods`(goods_id),
    CONSTRAINT chk_orders_quantity CHECK (quantity > 0),
    CONSTRAINT chk_orders_total CHECK (total_price >= 0)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `Favorite` (
    favorite_id INT PRIMARY KEY AUTO_INCREMENT,
    user_id INT NOT NULL,
    goods_id INT NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY uq_favorite_user_goods (user_id, goods_id),
    CONSTRAINT fk_favorite_user FOREIGN KEY (user_id) REFERENCES `User`(user_id) ON DELETE CASCADE,
    CONSTRAINT fk_favorite_goods FOREIGN KEY (goods_id) REFERENCES `Goods`(goods_id) ON DELETE CASCADE
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `Cart` (
    cart_id INT PRIMARY KEY AUTO_INCREMENT,
    user_id INT NOT NULL UNIQUE,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    CONSTRAINT fk_cart_user FOREIGN KEY (user_id) REFERENCES `User`(user_id) ON DELETE CASCADE
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `CartItem` (
    cart_item_id INT PRIMARY KEY AUTO_INCREMENT,
    cart_id INT NOT NULL,
    goods_id INT NOT NULL,
    quantity INT NOT NULL DEFAULT 1,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    UNIQUE KEY uq_cart_goods (cart_id, goods_id),
    CONSTRAINT fk_cart_item_cart FOREIGN KEY (cart_id) REFERENCES `Cart`(cart_id) ON DELETE CASCADE,
    CONSTRAINT fk_cart_item_goods FOREIGN KEY (goods_id) REFERENCES `Goods`(goods_id) ON DELETE CASCADE,
    CONSTRAINT chk_cart_item_quantity CHECK (quantity > 0)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `Message` (
    message_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    sender_id INT NOT NULL,
    receiver_id INT NOT NULL,
    goods_id INT NULL,
    body VARCHAR(1000) NOT NULL,
    is_read TINYINT(1) NOT NULL DEFAULT 0,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_message_conversation (sender_id, receiver_id, created_at),
    CONSTRAINT fk_message_sender FOREIGN KEY (sender_id) REFERENCES `User`(user_id) ON DELETE CASCADE,
    CONSTRAINT fk_message_receiver FOREIGN KEY (receiver_id) REFERENCES `User`(user_id) ON DELETE CASCADE,
    CONSTRAINT fk_message_goods FOREIGN KEY (goods_id) REFERENCES `Goods`(goods_id) ON DELETE SET NULL
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `ProductAsset` (
    asset_id INT PRIMARY KEY AUTO_INCREMENT,
    goods_id INT NOT NULL UNIQUE,
    local_path VARCHAR(500) NOT NULL,
    source_url VARCHAR(1000) NOT NULL,
    source_page VARCHAR(1000) DEFAULT 'https://unsplash.com/',
    license_name VARCHAR(100) NOT NULL DEFAULT 'Unsplash License',
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_asset_goods FOREIGN KEY (goods_id) REFERENCES `Goods`(goods_id) ON DELETE CASCADE
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

INSERT INTO `User` (username, email, password_hash, phone, status, preference_category) VALUES
    ('demo_user', 'demo_user@example.com', 'demo123', '07123456789', '正常', '电子数码'),
    ('lin', 'lin@example.com', 'campus123', '07123450001', '正常', '书籍教材'),
    ('maya', 'maya@example.com', 'campus123', '07123450002', '正常', '服饰鞋包'),
    ('alex', 'alex@example.com', 'campus123', '07123450003', '正常', '运动器材')
ON DUPLICATE KEY UPDATE
    phone = VALUES(phone),
    status = VALUES(status),
    preference_category = VALUES(preference_category);

INSERT INTO `Admin` (admin_name, password_hash, phone)
VALUES ('admin', 'admin123', '07000000000')
ON DUPLICATE KEY UPDATE phone = VALUES(phone);

INSERT INTO `Category` (name, description) VALUES
    ('书籍教材', 'Textbooks and course reading'),
    ('学习资料', 'Study tools and revision materials'),
    ('电子数码', 'Phones, computers and accessories'),
    ('宿舍用品', 'Everyday dormitory essentials'),
    ('服饰鞋包', 'Clothing, shoes and bags'),
    ('运动器材', 'Sports and fitness equipment'),
    ('代步工具', 'Bikes and campus transport'),
    ('美妆零食', 'Personal care and snacks')
ON DUPLICATE KEY UPDATE description = VALUES(description);

INSERT INTO `Goods`
    (user_id, category_id, title, description, price, stock, condition_level, image_path, status, sales_count)
SELECT
    u.user_id, c.category_id, seed.title, seed.description, seed.price, seed.stock,
    seed.condition_level, seed.image_path, '已上架', seed.sales_count
FROM (
    SELECT 'lin' seller, '书籍教材' category, '高等数学教材' title, '整洁无缺页，含少量重点标注。' description, 35.00 price, 8 stock, 8 condition_level, 'ui_assets/products/calculus_textbook.jpg' image_path, 28 sales_count
    UNION ALL SELECT 'lin', '书籍教材', 'C++ 程序设计', '适合程序设计入门与期末复习。', 25.00, 6, 8, 'ui_assets/products/cpp_textbook.jpg', 16
    UNION ALL SELECT 'lin', '学习资料', '雅思备考资料', '阅读、写作与词汇笔记整理完整。', 18.00, 12, 8, 'ui_assets/products/ielts_guide.jpg', 9
    UNION ALL SELECT 'lin', '学习资料', '科学计算器', '功能正常，适合数学与工程课程。', 45.00, 4, 9, 'ui_assets/products/calculator.jpg', 7
    UNION ALL SELECT 'demo_user', '电子数码', '二手 iPhone 12', '屏幕显示正常，电池与摄像功能良好。', 2100.00, 1, 8, 'ui_assets/products/iphone12.jpg', 6
    UNION ALL SELECT 'demo_user', '电子数码', '蓝牙耳机', '无线连接稳定，耳罩干净。', 99.00, 2, 8, 'ui_assets/products/bluetooth_headphones.jpg', 19
    UNION ALL SELECT 'demo_user', '电子数码', '轻薄笔记本电脑', '适合上课、编程和日常文档处理。', 3200.00, 1, 8, 'ui_assets/products/laptop.jpg', 4
    UNION ALL SELECT 'demo_user', '电子数码', '机械键盘', '按键手感清晰，所有键位工作正常。', 160.00, 5, 9, 'ui_assets/products/mechanical_keyboard.jpg', 12
    UNION ALL SELECT 'lin', '宿舍用品', '护眼台灯', '亮度可调，适合宿舍书桌使用。', 30.00, 3, 8, 'ui_assets/products/desk_lamp.jpg', 9
    UNION ALL SELECT 'lin', '宿舍用品', '宿舍收纳箱', '透明大容量收纳箱，方便整理衣物。', 22.50, 5, 8, 'ui_assets/products/storage_box.jpg', 3
    UNION ALL SELECT 'lin', '宿舍用品', '保温水杯', '密封良好，容量适合日常携带。', 28.00, 7, 9, 'ui_assets/products/water_bottle.jpg', 8
    UNION ALL SELECT 'lin', '宿舍用品', '桌面小风扇', '运行安静，多档风速可调。', 55.00, 4, 8, 'ui_assets/products/desk_fan.jpg', 6
    UNION ALL SELECT 'maya', '服饰鞋包', '九成新运动鞋', '轻便舒适，清洁后保存。', 120.00, 2, 9, 'ui_assets/products/sneakers.jpg', 10
    UNION ALL SELECT 'maya', '服饰鞋包', '校园双肩包', '电脑夹层完整，适合通勤上课。', 85.00, 3, 8, 'ui_assets/products/backpack.jpg', 14
    UNION ALL SELECT 'maya', '服饰鞋包', '秋季外套', '版型简洁，无明显磨损。', 150.00, 2, 8, 'ui_assets/products/coat.jpg', 5
    UNION ALL SELECT 'alex', '运动器材', '羽毛球拍', '重量轻，适合日常训练与社团活动。', 88.00, 4, 9, 'ui_assets/products/badminton_racket.jpg', 13
    UNION ALL SELECT 'alex', '运动器材', '篮球', '室内外均可使用，弹性正常。', 65.00, 3, 8, 'ui_assets/products/basketball.jpg', 11
    UNION ALL SELECT 'alex', '运动器材', '瑜伽垫', '防滑耐用，已清洁消毒。', 40.00, 6, 8, 'ui_assets/products/yoga_mat.jpg', 8
    UNION ALL SELECT 'alex', '代步工具', '校园自行车', '刹车与变速正常，适合校园通勤。', 280.00, 1, 7, 'ui_assets/products/bicycle.jpg', 7
    UNION ALL SELECT 'alex', '代步工具', '折叠滑板车', '可折叠便携，轮胎状态良好。', 420.00, 1, 8, 'ui_assets/products/scooter.jpg', 2
) AS seed
JOIN `User` u ON u.username = seed.seller
JOIN `Category` c ON c.name = seed.category
LEFT JOIN `Goods` existing ON existing.title = seed.title AND existing.is_deleted = 0
WHERE existing.goods_id IS NULL;

UPDATE `Goods`
SET image_path = CASE title
    WHEN '高等数学教材' THEN 'ui_assets/products/calculus_textbook.jpg'
    WHEN 'C++ 程序设计' THEN 'ui_assets/products/cpp_textbook.jpg'
    WHEN '雅思备考资料' THEN 'ui_assets/products/ielts_guide.jpg'
    WHEN '科学计算器' THEN 'ui_assets/products/calculator.jpg'
    WHEN '二手 iPhone 12' THEN 'ui_assets/products/iphone12.jpg'
    WHEN '蓝牙耳机' THEN 'ui_assets/products/bluetooth_headphones.jpg'
    WHEN '轻薄笔记本电脑' THEN 'ui_assets/products/laptop.jpg'
    WHEN '机械键盘' THEN 'ui_assets/products/mechanical_keyboard.jpg'
    WHEN '护眼台灯' THEN 'ui_assets/products/desk_lamp.jpg'
    WHEN '宿舍收纳箱' THEN 'ui_assets/products/storage_box.jpg'
    WHEN '保温水杯' THEN 'ui_assets/products/water_bottle.jpg'
    WHEN '桌面小风扇' THEN 'ui_assets/products/desk_fan.jpg'
    WHEN '九成新运动鞋' THEN 'ui_assets/products/sneakers.jpg'
    WHEN '校园双肩包' THEN 'ui_assets/products/backpack.jpg'
    WHEN '秋季外套' THEN 'ui_assets/products/coat.jpg'
    WHEN '羽毛球拍' THEN 'ui_assets/products/badminton_racket.jpg'
    WHEN '篮球' THEN 'ui_assets/products/basketball.jpg'
    WHEN '瑜伽垫' THEN 'ui_assets/products/yoga_mat.jpg'
    WHEN '校园自行车' THEN 'ui_assets/products/bicycle.jpg'
    WHEN '折叠滑板车' THEN 'ui_assets/products/scooter.jpg'
    ELSE image_path
END
WHERE title IN (
    '高等数学教材', 'C++ 程序设计', '雅思备考资料', '科学计算器',
    '二手 iPhone 12', '蓝牙耳机', '轻薄笔记本电脑', '机械键盘',
    '护眼台灯', '宿舍收纳箱', '保温水杯', '桌面小风扇',
    '九成新运动鞋', '校园双肩包', '秋季外套', '羽毛球拍',
    '篮球', '瑜伽垫', '校园自行车', '折叠滑板车'
);

INSERT INTO `ProductAsset` (goods_id, local_path, source_url, source_page, license_name)
SELECT g.goods_id, g.image_path, asset_seed.source_url, asset_seed.source_page, 'Unsplash License'
FROM (
    SELECT '高等数学教材' title, 'https://images.unsplash.com/photo-1495446815901-a7297e633e8d' source_url, 'https://unsplash.com/s/photos/textbook' source_page
    UNION ALL SELECT 'C++ 程序设计', 'https://images.unsplash.com/photo-1516321318423-f06f85e504b3', 'https://unsplash.com/s/photos/programming'
    UNION ALL SELECT '雅思备考资料', 'https://images.unsplash.com/photo-1434030216411-0b793f4b4173', 'https://unsplash.com/s/photos/studying'
    UNION ALL SELECT '科学计算器', 'https://images.unsplash.com/photo-1587145820266-a5951ee6f620', 'https://unsplash.com/s/photos/calculator'
    UNION ALL SELECT '二手 iPhone 12', 'https://images.unsplash.com/photo-1511707171634-5f897ff02aa9', 'https://unsplash.com/s/photos/iphone'
    UNION ALL SELECT '蓝牙耳机', 'https://images.unsplash.com/photo-1505740420928-5e560c06d30e', 'https://unsplash.com/s/photos/headphones'
    UNION ALL SELECT '轻薄笔记本电脑', 'https://images.unsplash.com/photo-1496181133206-80ce9b88a853', 'https://unsplash.com/s/photos/laptop'
    UNION ALL SELECT '机械键盘', 'https://images.unsplash.com/photo-1587829741301-dc798b83add3', 'https://unsplash.com/s/photos/keyboard'
    UNION ALL SELECT '护眼台灯', 'https://images.unsplash.com/photo-1507473885765-e6ed057f782c', 'https://unsplash.com/s/photos/desk-lamp'
    UNION ALL SELECT '宿舍收纳箱', 'https://images.unsplash.com/photo-1786396798357-ac8ba6704d89', 'https://unsplash.com/photos/labeled-storage-bins-on-wire-shelving-yRLAF1UKiXU'
    UNION ALL SELECT '保温水杯', 'https://images.unsplash.com/photo-1602143407151-7111542de6e8', 'https://unsplash.com/s/photos/water-bottle'
    UNION ALL SELECT '桌面小风扇', 'https://images.unsplash.com/photo-1568392816241-44d223d4d490', 'https://unsplash.com/photos/turned-on-white-desk-fan-FSIT7PcgXC0'
    UNION ALL SELECT '九成新运动鞋', 'https://images.unsplash.com/photo-1542291026-7eec264c27ff', 'https://unsplash.com/s/photos/sneakers'
    UNION ALL SELECT '校园双肩包', 'https://images.unsplash.com/photo-1553062407-98eeb64c6a62', 'https://unsplash.com/s/photos/backpack'
    UNION ALL SELECT '秋季外套', 'https://images.unsplash.com/photo-1551028719-00167b16eac5', 'https://unsplash.com/s/photos/jacket'
    UNION ALL SELECT '羽毛球拍', 'https://images.unsplash.com/photo-1722003180803-577efd6d2ecc', 'https://unsplash.com/photos/a-badminton-racket-with-a-white-shuttle-on-it-wieTrtA9v6I'
    UNION ALL SELECT '篮球', 'https://images.unsplash.com/photo-1546519638-68e109498ffc', 'https://unsplash.com/s/photos/basketball'
    UNION ALL SELECT '瑜伽垫', 'https://images.unsplash.com/photo-1601925260368-ae2f83cf8b7f', 'https://unsplash.com/s/photos/yoga-mat'
    UNION ALL SELECT '校园自行车', 'https://images.unsplash.com/photo-1529422643029-d4585747aaf2', 'https://unsplash.com/s/photos/bicycle'
    UNION ALL SELECT '折叠滑板车', 'https://images.unsplash.com/photo-1729824253691-3b7b4e4303f0', 'https://unsplash.com/photos/a-scooter-is-parked-on-the-side-of-the-road-D_HJgTESqls'
) AS asset_seed
JOIN `Goods` g ON g.title = asset_seed.title AND g.is_deleted = 0
ON DUPLICATE KEY UPDATE
    local_path = VALUES(local_path),
    source_url = VALUES(source_url),
    source_page = VALUES(source_page),
    license_name = VALUES(license_name);
