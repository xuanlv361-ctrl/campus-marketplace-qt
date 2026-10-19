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

CREATE TABLE IF NOT EXISTS `Campus` (
    campus_id INT PRIMARY KEY AUTO_INCREMENT,
    name_zh VARCHAR(80) NOT NULL UNIQUE,
    name_en VARCHAR(120) NOT NULL,
    city VARCHAR(80) NOT NULL DEFAULT '',
    is_active TINYINT(1) NOT NULL DEFAULT 1,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `UserAddress` (
    address_id INT PRIMARY KEY AUTO_INCREMENT,
    user_id INT NOT NULL,
    campus_id INT NULL,
    label VARCHAR(40) NOT NULL DEFAULT '默认地址',
    contact_name VARCHAR(80) NOT NULL,
    contact_phone VARCHAR(30) NOT NULL,
    building VARCHAR(120) NOT NULL,
    room VARCHAR(60) DEFAULT '',
    detail VARCHAR(255) DEFAULT '',
    is_default TINYINT(1) NOT NULL DEFAULT 0,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    UNIQUE KEY uq_user_address_label (user_id, label),
    INDEX idx_address_campus (campus_id),
    CONSTRAINT fk_address_user FOREIGN KEY (user_id) REFERENCES `User`(user_id) ON DELETE CASCADE,
    CONSTRAINT fk_address_campus FOREIGN KEY (campus_id) REFERENCES `Campus`(campus_id) ON DELETE SET NULL
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `GoodsImage` (
    image_id INT PRIMARY KEY AUTO_INCREMENT,
    goods_id INT NOT NULL,
    image_path VARCHAR(500) NOT NULL,
    source_url VARCHAR(1000) DEFAULT '',
    source_page VARCHAR(1000) DEFAULT '',
    license_name VARCHAR(100) DEFAULT '',
    alt_text_zh VARCHAR(255) DEFAULT '',
    alt_text_en VARCHAR(255) DEFAULT '',
    display_order INT NOT NULL DEFAULT 0,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY uq_goods_image_order (goods_id, display_order),
    CONSTRAINT fk_goods_image_goods FOREIGN KEY (goods_id) REFERENCES `Goods`(goods_id) ON DELETE CASCADE
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `Payment` (
    payment_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    order_id INT NOT NULL UNIQUE,
    payment_method VARCHAR(30) NOT NULL DEFAULT '校园卡',
    amount DECIMAL(10, 2) NOT NULL,
    status VARCHAR(20) NOT NULL DEFAULT '待支付',
    transaction_no VARCHAR(80) NULL UNIQUE,
    paid_at TIMESTAMP NULL,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    CONSTRAINT fk_payment_order FOREIGN KEY (order_id) REFERENCES `Orders`(order_id) ON DELETE CASCADE,
    CONSTRAINT chk_payment_amount CHECK (amount >= 0)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `Shipment` (
    shipment_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    order_id INT NOT NULL UNIQUE,
    delivery_type VARCHAR(30) NOT NULL DEFAULT '校园自提',
    pickup_location VARCHAR(255) DEFAULT '',
    tracking_no VARCHAR(100) NULL UNIQUE,
    status VARCHAR(20) NOT NULL DEFAULT '待发货',
    shipped_at TIMESTAMP NULL,
    completed_at TIMESTAMP NULL,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    CONSTRAINT fk_shipment_order FOREIGN KEY (order_id) REFERENCES `Orders`(order_id) ON DELETE CASCADE
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `Review` (
    review_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    order_id INT NOT NULL UNIQUE,
    reviewer_id INT NOT NULL,
    goods_id INT NOT NULL,
    rating TINYINT NOT NULL,
    content VARCHAR(1000) DEFAULT '',
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_review_goods_created (goods_id, created_at),
    CONSTRAINT fk_review_order FOREIGN KEY (order_id) REFERENCES `Orders`(order_id) ON DELETE CASCADE,
    CONSTRAINT fk_review_user FOREIGN KEY (reviewer_id) REFERENCES `User`(user_id) ON DELETE CASCADE,
    CONSTRAINT fk_review_goods FOREIGN KEY (goods_id) REFERENCES `Goods`(goods_id) ON DELETE CASCADE,
    CONSTRAINT chk_review_rating CHECK (rating BETWEEN 1 AND 5)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `Notification` (
    notification_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    user_id INT NOT NULL,
    type VARCHAR(40) NOT NULL DEFAULT '系统通知',
    title VARCHAR(160) NOT NULL,
    content VARCHAR(1000) NOT NULL,
    related_entity VARCHAR(40) DEFAULT '',
    related_id BIGINT NULL,
    dedupe_key VARCHAR(120) NULL UNIQUE,
    is_read TINYINT(1) NOT NULL DEFAULT 0,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_notification_user_read (user_id, is_read, created_at),
    CONSTRAINT fk_notification_user FOREIGN KEY (user_id) REFERENCES `User`(user_id) ON DELETE CASCADE
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `GoodsReport` (
    report_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    goods_id INT NOT NULL,
    reporter_id INT NOT NULL,
    reason VARCHAR(80) NOT NULL,
    description VARCHAR(1000) DEFAULT '',
    status VARCHAR(20) NOT NULL DEFAULT '待处理',
    handled_by INT NULL,
    handled_at TIMESTAMP NULL,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY uq_reporter_goods (reporter_id, goods_id),
    INDEX idx_report_status_created (status, created_at),
    CONSTRAINT fk_report_goods FOREIGN KEY (goods_id) REFERENCES `Goods`(goods_id) ON DELETE CASCADE,
    CONSTRAINT fk_report_user FOREIGN KEY (reporter_id) REFERENCES `User`(user_id) ON DELETE CASCADE,
    CONSTRAINT fk_report_admin FOREIGN KEY (handled_by) REFERENCES `Admin`(admin_id) ON DELETE SET NULL
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `GoodsView` (
    view_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    goods_id INT NOT NULL,
    user_id INT NULL,
    session_key VARCHAR(120) DEFAULT '',
    viewed_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_goods_view_goods_time (goods_id, viewed_at),
    INDEX idx_goods_view_user_time (user_id, viewed_at),
    CONSTRAINT fk_goods_view_goods FOREIGN KEY (goods_id) REFERENCES `Goods`(goods_id) ON DELETE CASCADE,
    CONSTRAINT fk_goods_view_user FOREIGN KEY (user_id) REFERENCES `User`(user_id) ON DELETE SET NULL
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `SearchHistory` (
    search_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    user_id INT NOT NULL,
    keyword VARCHAR(160) NOT NULL DEFAULT '',
    category VARCHAR(80) NOT NULL DEFAULT '',
    min_price DECIMAL(10, 2) NULL,
    max_price DECIMAL(10, 2) NULL,
    searched_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_search_user_time (user_id, searched_at),
    CONSTRAINT fk_search_user FOREIGN KEY (user_id) REFERENCES `User`(user_id) ON DELETE CASCADE
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `AdminAuditLog` (
    audit_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    admin_id INT NULL,
    action VARCHAR(80) NOT NULL,
    entity_type VARCHAR(50) NOT NULL,
    entity_id VARCHAR(80) DEFAULT '',
    details TEXT,
    ip_address VARCHAR(64) DEFAULT '',
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_audit_admin_time (admin_id, created_at),
    INDEX idx_audit_entity (entity_type, entity_id),
    CONSTRAINT fk_audit_admin FOREIGN KEY (admin_id) REFERENCES `Admin`(admin_id) ON DELETE SET NULL
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

CREATE OR REPLACE VIEW view_order_detail AS
SELECT
    o.order_id,
    buyer.username AS buyer,
    seller.username AS seller,
    g.title AS goods_name,
    c.name AS category_name,
    o.quantity,
    o.total_price,
    o.status AS order_status,
    COALESCE(p.status, '未创建') AS payment_status,
    COALESCE(s.status, '未创建') AS shipment_status,
    COALESCE(s.delivery_type, '') AS delivery_type,
    o.created_at,
    o.updated_at
FROM `Orders` o
JOIN `User` buyer ON buyer.user_id = o.user_id
JOIN `Goods` g ON g.goods_id = o.goods_id
JOIN `User` seller ON seller.user_id = g.user_id
JOIN `Category` c ON c.category_id = g.category_id
LEFT JOIN `Payment` p ON p.order_id = o.order_id
LEFT JOIN `Shipment` s ON s.order_id = o.order_id;

CREATE OR REPLACE VIEW view_category_sales AS
SELECT
    c.category_id,
    c.name AS category_name,
    COUNT(g.goods_id) AS listing_count,
    COALESCE(SUM(g.stock), 0) AS available_stock,
    COALESCE(SUM(g.sales_count), 0) AS units_sold,
    COALESCE(SUM(g.sales_count * g.price), 0) AS estimated_revenue
FROM `Category` c
LEFT JOIN `Goods` g ON g.category_id = c.category_id AND g.is_deleted = 0
GROUP BY c.category_id, c.name;

CREATE OR REPLACE VIEW view_goods_engagement AS
SELECT
    g.goods_id,
    g.title,
    COALESCE(f.favorite_count, 0) AS favorite_count,
    COALESCE(v.view_count, 0) AS view_count,
    COALESCE(r.review_count, 0) AS review_count,
    COALESCE(r.average_rating, 0) AS average_rating
FROM `Goods` g
LEFT JOIN (
    SELECT goods_id, COUNT(*) AS favorite_count
    FROM `Favorite`
    GROUP BY goods_id
) f ON f.goods_id = g.goods_id
LEFT JOIN (
    SELECT goods_id, COUNT(*) AS view_count
    FROM `GoodsView`
    GROUP BY goods_id
) v ON v.goods_id = g.goods_id
LEFT JOIN (
    SELECT goods_id, COUNT(*) AS review_count, ROUND(AVG(rating), 2) AS average_rating
    FROM `Review`
    GROUP BY goods_id
) r ON r.goods_id = g.goods_id
WHERE g.is_deleted = 0;

INSERT INTO `User` (username, email, password_hash, phone, status, preference_category) VALUES
    ('demo_user', 'demo_user@example.com', 'demo123', '07123456789', '正常', '电子数码'),
    ('lin', 'lin@example.com', 'campus123', '07123450001', '正常', '书籍教材'),
    ('maya', 'maya@example.com', 'campus123', '07123450002', '正常', '服饰鞋包'),
    ('alex', 'alex@example.com', 'campus123', '07123450003', '正常', '运动器材'),
    ('sophie', 'sophie@example.com', 'campus123', '07123450004', '正常', '小家电'),
    ('daniel', 'daniel@example.com', 'campus123', '07123450005', '正常', '办公设备'),
    ('emma', 'emma@example.com', 'campus123', '07123450006', '正常', '电子数码'),
    ('oliver', 'oliver@example.com', 'campus123', '07123450007', '正常', '代步工具'),
    ('chen', 'chen@example.com', 'campus123', '07123450008', '正常', '电子数码'),
    ('priya', 'priya@example.com', 'campus123', '07123450009', '正常', '音乐乐器')
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
    ('美妆零食', 'Personal care and snacks'),
    ('办公设备', 'Monitors, printers and study workspace equipment'),
    ('小家电', 'Compact appliances suitable for student accommodation'),
    ('音乐乐器', 'Musical instruments and creative equipment')
ON DUPLICATE KEY UPDATE description = VALUES(description);

INSERT INTO `Campus` (name_zh, name_en, city, is_active) VALUES
    ('主校区', 'Main Campus', 'Birmingham', 1),
    ('北校区', 'North Campus', 'Birmingham', 1),
    ('南校区', 'South Campus', 'Birmingham', 1)
ON DUPLICATE KEY UPDATE
    name_en = VALUES(name_en),
    city = VALUES(city),
    is_active = VALUES(is_active);

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
    UNION ALL SELECT 'emma', '电子数码', 'iPad Air 平板', '屏幕清晰，支持触控笔，适合课堂笔记和阅读。', 1650.00, 1, 8, 'ui_assets/products/tablet_ipad.jpg', 7
    UNION ALL SELECT 'daniel', '办公设备', '24英寸显示器', '1080P 显示正常，配原装电源线与 HDMI 线。', 480.00, 2, 8, 'ui_assets/products/monitor.jpg', 5
    UNION ALL SELECT 'chen', '办公设备', '无线办公鼠标', '人体工学设计，蓝牙和接收器连接均正常。', 75.00, 6, 9, 'ui_assets/products/wireless_mouse.jpg', 12
    UNION ALL SELECT 'daniel', '办公设备', '便携打印机', '打印与扫描功能正常，适合宿舍和社团使用。', 260.00, 2, 8, 'ui_assets/products/compact_printer.jpg', 3
    UNION ALL SELECT 'sophie', '小家电', '宿舍电热水壶', '自动断电功能正常，已完成除垢清洁。', 65.00, 3, 8, 'ui_assets/products/electric_kettle.jpg', 8
    UNION ALL SELECT 'maya', '服饰鞋包', '轻薄连帽卫衣', '宽松版型，面料柔软，无明显起球。', 90.00, 4, 8, 'ui_assets/products/hoodie.jpg', 7
    UNION ALL SELECT 'alex', '运动器材', '网球拍', '拍框无变形，手胶已更换，适合入门训练。', 110.00, 2, 8, 'ui_assets/products/tennis_racket.jpg', 4
    UNION ALL SELECT 'alex', '运动器材', '六角哑铃一对', '一对固定重量哑铃，适合宿舍基础力量训练。', 85.00, 3, 8, 'ui_assets/products/dumbbells.jpg', 6
    UNION ALL SELECT 'oliver', '代步工具', '校园长板', '轮组顺滑，板面牢固，适合平地代步。', 160.00, 1, 7, 'ui_assets/products/skateboard.jpg', 2
    UNION ALL SELECT 'priya', '音乐乐器', '民谣木吉他', '音准稳定，琴颈顺直，附基础琴包。', 520.00, 1, 8, 'ui_assets/products/acoustic_guitar.jpg', 3
    UNION ALL SELECT 'emma', '电子数码', '复古拍立得相机', '快门与闪光灯工作正常，适合社团活动记录。', 340.00, 1, 7, 'ui_assets/products/instant_camera.jpg', 5
    UNION ALL SELECT 'chen', '电子数码', 'USB-C扩展坞', '多接口扩展，适合笔记本和平板连接外设。', 95.00, 5, 9, 'ui_assets/products/usb_c_hub.jpg', 10
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
    WHEN 'iPad Air 平板' THEN 'ui_assets/products/tablet_ipad.jpg'
    WHEN '24英寸显示器' THEN 'ui_assets/products/monitor.jpg'
    WHEN '无线办公鼠标' THEN 'ui_assets/products/wireless_mouse.jpg'
    WHEN '便携打印机' THEN 'ui_assets/products/compact_printer.jpg'
    WHEN '宿舍电热水壶' THEN 'ui_assets/products/electric_kettle.jpg'
    WHEN '轻薄连帽卫衣' THEN 'ui_assets/products/hoodie.jpg'
    WHEN '网球拍' THEN 'ui_assets/products/tennis_racket.jpg'
    WHEN '六角哑铃一对' THEN 'ui_assets/products/dumbbells.jpg'
    WHEN '校园长板' THEN 'ui_assets/products/skateboard.jpg'
    WHEN '民谣木吉他' THEN 'ui_assets/products/acoustic_guitar.jpg'
    WHEN '复古拍立得相机' THEN 'ui_assets/products/instant_camera.jpg'
    WHEN 'USB-C扩展坞' THEN 'ui_assets/products/usb_c_hub.jpg'
    ELSE image_path
END
WHERE title IN (
    '高等数学教材', 'C++ 程序设计', '雅思备考资料', '科学计算器',
    '二手 iPhone 12', '蓝牙耳机', '轻薄笔记本电脑', '机械键盘',
    '护眼台灯', '宿舍收纳箱', '保温水杯', '桌面小风扇',
    '九成新运动鞋', '校园双肩包', '秋季外套', '羽毛球拍',
    '篮球', '瑜伽垫', '校园自行车', '折叠滑板车',
    'iPad Air 平板', '24英寸显示器', '无线办公鼠标', '便携打印机',
    '宿舍电热水壶', '轻薄连帽卫衣', '网球拍', '六角哑铃一对',
    '校园长板', '民谣木吉他', '复古拍立得相机', 'USB-C扩展坞'
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
    UNION ALL SELECT 'iPad Air 平板', 'https://images.unsplash.com/photo-1708765275942-091b2e04905c', 'https://unsplash.com/photos/a-tablet-computer-sitting-on-top-of-a-wooden-table-w3mSws0IAU4'
    UNION ALL SELECT '24英寸显示器', 'https://images.unsplash.com/photo-1616440347437-b1c73416efc2', 'https://unsplash.com/photos/black-flat-screen-computer-monitor-on-white-wooden-desk-Gvm2wM3V5PA'
    UNION ALL SELECT '无线办公鼠标', 'https://images.unsplash.com/photo-1770479592099-464241a3ad3a', 'https://unsplash.com/photos/a-black-computer-mouse-on-a-white-desk--0Wqx50_W7g'
    UNION ALL SELECT '便携打印机', 'https://images.unsplash.com/photo-1613395450289-e560907d9308', 'https://unsplash.com/photos/black-and-gray-printer-on-white-table-8r1ZlqqGxMU'
    UNION ALL SELECT '宿舍电热水壶', 'https://images.unsplash.com/photo-1725419281142-3f89b330b907', 'https://unsplash.com/photos/a-white-toaster-sitting-on-top-of-a-counter-ja4Tq8nIq3E'
    UNION ALL SELECT '轻薄连帽卫衣', 'https://images.unsplash.com/photo-1499972777470-6a932ea55420', 'https://unsplash.com/photos/black-hoodie-7k73_NUaXDg'
    UNION ALL SELECT '网球拍', 'https://images.unsplash.com/photo-1599409096020-e465eb5a93e2', 'https://unsplash.com/photos/a-close-up-of-a-tennis-racket-on-a-blue-surface-E0zwjUXdZtE'
    UNION ALL SELECT '六角哑铃一对', 'https://images.unsplash.com/photo-1562771242-a02d9090c90c', 'https://unsplash.com/photos/pair-of-black-dumbbells-zEAX0E0KJxs'
    UNION ALL SELECT '校园长板', 'https://images.unsplash.com/photo-1511190874914-2606678a0693', 'https://unsplash.com/photos/black-wooden-skateboard-on-floor-ff_eDw14Jkg'
    UNION ALL SELECT '民谣木吉他', 'https://images.unsplash.com/photo-1573603088895-d399fbee9653', 'https://unsplash.com/photos/acoustic-guitar-on-sofa-ZIbrlLp63PY'
    UNION ALL SELECT '复古拍立得相机', 'https://images.unsplash.com/photo-1462998071405-47e34dec61db', 'https://unsplash.com/photos/polaroid-instant-camera-on-box-uvrr8j6gTeU'
    UNION ALL SELECT 'USB-C扩展坞', 'https://images.unsplash.com/photo-1760376789478-c1023d2dc007', 'https://unsplash.com/photos/black-usb-hub-with-multiple-ports-and-switches-b1UEh_5dKLY'
) AS asset_seed
JOIN `Goods` g ON g.title = asset_seed.title AND g.is_deleted = 0
ON DUPLICATE KEY UPDATE
    local_path = VALUES(local_path),
    source_url = VALUES(source_url),
    source_page = VALUES(source_page),
    license_name = VALUES(license_name);

INSERT INTO `GoodsImage`
    (goods_id, image_path, source_url, source_page, license_name, alt_text_zh, alt_text_en, display_order)
SELECT
    g.goods_id,
    pa.local_path,
    pa.source_url,
    pa.source_page,
    pa.license_name,
    CONCAT(g.title, ' 商品图片'),
    CONCAT('Product image for ', g.title),
    0
FROM `ProductAsset` pa
JOIN `Goods` g ON g.goods_id = pa.goods_id
ON DUPLICATE KEY UPDATE
    image_path = VALUES(image_path),
    source_url = VALUES(source_url),
    source_page = VALUES(source_page),
    license_name = VALUES(license_name),
    alt_text_zh = VALUES(alt_text_zh),
    alt_text_en = VALUES(alt_text_en);

INSERT INTO `UserAddress`
    (user_id, campus_id, label, contact_name, contact_phone, building, room, detail, is_default)
SELECT
    u.user_id,
    c.campus_id,
    address_seed.label,
    address_seed.contact_name,
    u.phone,
    address_seed.building,
    address_seed.room,
    address_seed.detail,
    address_seed.is_default
FROM (
    SELECT 'demo_user' username, '主校区' campus, '宿舍' label, 'Demo User' contact_name, 'Maple Hall' building, 'A-203' room, 'Reception collection point' detail, 1 is_default
    UNION ALL SELECT 'lin', '主校区', '宿舍', 'Lin', 'Oak Hall', 'B-118', 'Please message before arrival', 1
    UNION ALL SELECT 'maya', '北校区', '工作室', 'Maya', 'Design Building', '2-14', 'Weekday collection', 1
    UNION ALL SELECT 'alex', '南校区', '体育馆', 'Alex', 'Sports Centre', 'Reception', 'Evening collection preferred', 1
    UNION ALL SELECT 'emma', '主校区', '宿舍', 'Emma', 'Cedar Hall', 'C-305', 'Leave with reception when confirmed', 1
) AS address_seed
JOIN `User` u ON u.username = address_seed.username
JOIN `Campus` c ON c.name_zh = address_seed.campus
ON DUPLICATE KEY UPDATE
    campus_id = VALUES(campus_id),
    contact_name = VALUES(contact_name),
    contact_phone = VALUES(contact_phone),
    building = VALUES(building),
    room = VALUES(room),
    detail = VALUES(detail),
    is_default = VALUES(is_default);

INSERT INTO `Favorite` (user_id, goods_id)
SELECT u.user_id, g.goods_id
FROM (
    SELECT 'demo_user' username, 'iPad Air 平板' title
    UNION ALL SELECT 'demo_user', '24英寸显示器'
    UNION ALL SELECT 'lin', '便携打印机'
    UNION ALL SELECT 'maya', '复古拍立得相机'
    UNION ALL SELECT 'alex', '校园长板'
    UNION ALL SELECT 'sophie', '宿舍电热水壶'
    UNION ALL SELECT 'daniel', '无线办公鼠标'
    UNION ALL SELECT 'priya', '民谣木吉他'
) AS favorite_seed
JOIN `User` u ON u.username = favorite_seed.username
JOIN `Goods` g ON g.title = favorite_seed.title AND g.is_deleted = 0
ON DUPLICATE KEY UPDATE goods_id = VALUES(goods_id);

INSERT INTO `Cart` (user_id)
SELECT user_id
FROM `User`
WHERE username IN ('demo_user', 'lin', 'maya', 'alex')
ON DUPLICATE KEY UPDATE user_id = VALUES(user_id);

INSERT INTO `CartItem` (cart_id, goods_id, quantity)
SELECT c.cart_id, g.goods_id, cart_seed.quantity
FROM (
    SELECT 'demo_user' username, '无线办公鼠标' title, 1 quantity
    UNION ALL SELECT 'demo_user', 'USB-C扩展坞', 1
    UNION ALL SELECT 'lin', '便携打印机', 1
    UNION ALL SELECT 'maya', '轻薄连帽卫衣', 1
    UNION ALL SELECT 'alex', '六角哑铃一对', 1
) AS cart_seed
JOIN `User` u ON u.username = cart_seed.username
JOIN `Cart` c ON c.user_id = u.user_id
JOIN `Goods` g ON g.title = cart_seed.title AND g.is_deleted = 0
ON DUPLICATE KEY UPDATE quantity = VALUES(quantity);

INSERT INTO `Orders` (user_id, goods_id, quantity, total_price, status)
SELECT
    u.user_id,
    g.goods_id,
    order_seed.quantity,
    g.price * order_seed.quantity,
    order_seed.status
FROM (
    SELECT 'demo_user' buyer, 'USB-C扩展坞' title, 1 quantity, '已完成' status
    UNION ALL SELECT 'sophie', '轻薄连帽卫衣', 1, '已完成'
    UNION ALL SELECT 'daniel', '24英寸显示器', 1, '未完成'
    UNION ALL SELECT 'emma', 'iPad Air 平板', 1, '已完成'
    UNION ALL SELECT 'oliver', '校园长板', 1, '未完成'
    UNION ALL SELECT 'priya', '便携打印机', 1, '已取消'
    UNION ALL SELECT 'chen', '无线办公鼠标', 2, '已完成'
    UNION ALL SELECT 'maya', '网球拍', 1, '已完成'
) AS order_seed
JOIN `User` u ON u.username = order_seed.buyer
JOIN `Goods` g ON g.title = order_seed.title AND g.is_deleted = 0
LEFT JOIN `Orders` existing ON existing.user_id = u.user_id AND existing.goods_id = g.goods_id
WHERE existing.order_id IS NULL;

INSERT INTO `Payment` (order_id, payment_method, amount, status, transaction_no, paid_at)
SELECT
    o.order_id,
    CASE WHEN MOD(o.order_id, 2) = 0 THEN '校园卡' ELSE '银行卡' END,
    o.total_price,
    CASE WHEN o.status = '已完成' THEN '已支付' ELSE '待支付' END,
    CASE WHEN o.status = '已完成' THEN CONCAT('SEED-', LPAD(o.order_id, 10, '0')) ELSE NULL END,
    CASE WHEN o.status = '已完成' THEN o.created_at ELSE NULL END
FROM `Orders` o
JOIN `Goods` g ON g.goods_id = o.goods_id
WHERE g.title IN (
    'USB-C扩展坞', '轻薄连帽卫衣', '24英寸显示器', 'iPad Air 平板',
    '校园长板', '无线办公鼠标', '网球拍'
)
AND o.status <> '已取消'
ON DUPLICATE KEY UPDATE
    amount = VALUES(amount),
    status = VALUES(status),
    paid_at = VALUES(paid_at);

INSERT INTO `Shipment` (order_id, delivery_type, pickup_location, status, shipped_at, completed_at)
SELECT
    o.order_id,
    CASE WHEN MOD(o.order_id, 2) = 0 THEN '校园自提' ELSE '当面交易' END,
    CASE WHEN MOD(o.order_id, 2) = 0 THEN 'Main Campus Library' ELSE 'Student Centre' END,
    CASE WHEN o.status = '已完成' THEN '已完成' ELSE '待发货' END,
    CASE WHEN o.status = '已完成' THEN o.created_at ELSE NULL END,
    CASE WHEN o.status = '已完成' THEN o.updated_at ELSE NULL END
FROM `Orders` o
JOIN `Goods` g ON g.goods_id = o.goods_id
WHERE g.title IN (
    'USB-C扩展坞', '轻薄连帽卫衣', '24英寸显示器', 'iPad Air 平板',
    '校园长板', '无线办公鼠标', '网球拍'
)
AND o.status <> '已取消'
ON DUPLICATE KEY UPDATE
    delivery_type = VALUES(delivery_type),
    pickup_location = VALUES(pickup_location),
    status = VALUES(status),
    shipped_at = VALUES(shipped_at),
    completed_at = VALUES(completed_at);

INSERT INTO `Review` (order_id, reviewer_id, goods_id, rating, content)
SELECT
    o.order_id,
    o.user_id,
    o.goods_id,
    CASE WHEN MOD(o.order_id, 3) = 0 THEN 4 ELSE 5 END,
    CASE
        WHEN g.title = 'iPad Air 平板' THEN '成色与描述一致，课堂使用很方便。'
        WHEN g.title = '无线办公鼠标' THEN '连接稳定，握持舒适。'
        WHEN g.title = '网球拍' THEN '拍框状态很好，交接顺利。'
        ELSE '商品状态良好，交易过程顺利。'
    END
FROM `Orders` o
JOIN `Goods` g ON g.goods_id = o.goods_id
LEFT JOIN `Review` existing ON existing.order_id = o.order_id
WHERE o.status = '已完成'
  AND existing.review_id IS NULL
  AND g.title IN ('USB-C扩展坞', '轻薄连帽卫衣', 'iPad Air 平板', '无线办公鼠标', '网球拍');

INSERT INTO `Notification`
    (user_id, type, title, content, related_entity, related_id, dedupe_key, is_read)
SELECT
    u.user_id,
    notification_seed.type,
    notification_seed.title,
    notification_seed.content,
    notification_seed.related_entity,
    g.goods_id,
    notification_seed.dedupe_key,
    notification_seed.is_read
FROM (
    SELECT 'emma' username, '商品动态' type, '商品已上架' title, '你的 iPad Air 平板 已通过审核并上架。' content, 'Goods' related_entity, 'iPad Air 平板' goods_title, 'seed-goods-ipad-approved' dedupe_key, 1 is_read
    UNION ALL SELECT 'daniel', '订单动态', '收到新的购买请求', '你的 24英寸显示器 收到一笔待处理订单。', 'Goods', '24英寸显示器', 'seed-order-monitor-created', 0
    UNION ALL SELECT 'chen', '交易完成', '订单已完成', '无线办公鼠标 的交易已经完成。', 'Goods', '无线办公鼠标', 'seed-order-mouse-completed', 0
    UNION ALL SELECT 'priya', '商品动态', '商品已上架', '你的 民谣木吉他 已通过审核并上架。', 'Goods', '民谣木吉他', 'seed-goods-guitar-approved', 1
) AS notification_seed
JOIN `User` u ON u.username = notification_seed.username
JOIN `Goods` g ON g.title = notification_seed.goods_title AND g.is_deleted = 0
ON DUPLICATE KEY UPDATE
    title = VALUES(title),
    content = VALUES(content),
    related_id = VALUES(related_id),
    is_read = VALUES(is_read);
