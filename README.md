# Campus Marketplace

A desktop second-hand marketplace for university communities, built with modern C++ and Qt Widgets. The application supports the complete journey from discovering and listing pre-owned items to purchasing, order tracking, moderation and operational reporting.

The project combines a warm, consumer-facing marketplace experience with a practical administration dashboard. It was designed to demonstrate desktop UI engineering, relational data modelling and end-to-end business workflow implementation.

## Highlights

- Responsive Qt Widgets interface with reusable QSS styling and card-based marketplace layouts
- User registration and authentication backed by MySQL
- Product discovery through search, category filters, price ranges and personalised recommendations
- Product detail views, stock-aware purchasing and a quantity-based shopping cart
- Purchase history and order status tracking
- Seller workflow for submitting second-hand listings for administrator review
- Administrator dashboard for products, orders, users and sales statistics
- Product approval, rejection, listing status management and destructive-action confirmations
- Shared data access layer that keeps stock, sales, orders and reporting in sync
- Data visualisation for category sales and live operational metrics

## Technology Stack

| Area | Technology |
| --- | --- |
| Language | C++17 |
| Desktop UI | Qt 6 Widgets, signals and slots, QSS |
| Database | MySQL 8.0 |
| Database integration | MySQL C API and Qt SQL module |
| Build system | CMake 3.16+ |
| Development environment | Qt Creator, MinGW 64-bit |

## User Experience

### Marketplace users

- Create an account and sign in
- Browse product cards in a scrollable marketplace layout
- Search and filter by keyword, category and price
- View product details, availability and condition
- Add products to the cart, adjust quantities and purchase selected items
- Review purchase history and current order status
- Submit personal items for sale and track the moderation status

### Administrators

- Review live counts from the dashboard
- Search, create, edit, approve, reject and remove products
- Manage stock, sales and listing status
- Search and update orders
- Manage user accounts, status and category preferences
- Analyse platform activity through KPI cards and category sales charts

## Architecture

The application uses window-specific Qt classes for presentation and navigation. `AccountStore` provides a shared data-access layer for users, products, carts, purchases and orders. This keeps customer and administrator views consistent: a successful purchase updates stock, sales, order history and statistics through the same source of truth.

```text
Qt Widgets UI
  |-- Marketplace and seller workflows
  |-- Cart and purchase history
  |-- Administration dashboard
  `-- Management and statistics windows
              |
         AccountStore
              |
          MySQL 8.0
```

## Build and Run

### Prerequisites

- Qt 6.9 or a compatible Qt 6 release with Widgets and SQL components
- CMake 3.16 or later
- A C++17 compiler such as MinGW 64-bit
- MySQL Server 8.0 and the MySQL client development files

### Configure the database

Import the included schema, then set the following environment variables in Qt Creator under **Projects > Run > Environment**:

```bash
mysql -u root -p < database/schema.sql
```

```text
SECONDHAND_DB_HOST=127.0.0.1
SECONDHAND_DB_PORT=3306
SECONDHAND_DB_NAME=shop
SECONDHAND_DB_USER=root
SECONDHAND_DB_PASSWORD=your_local_password
```

The application deliberately keeps credentials outside source control. See `.env.example` for the full configuration template.

The schema includes demonstration accounts for local development only:

- User: `demo_user` / `demo123`
- Administrator: `admin` / `admin123`

These sample credentials are intentionally non-production data. Real deployments should hash passwords and remove or replace the seed accounts.

### Configure and build

```bash
cmake -S . -B build -DMYSQL_ROOT="C:/Program Files/MySQL/MySQL Server 8.0"
cmake --build build
```

For the smoothest Windows setup, open the folder directly in Qt Creator, select a Qt 6 MinGW kit, configure `MYSQL_ROOT` if MySQL is installed elsewhere, and build the project.

## Engineering Notes

- SQL text uses UTF-8/UTF-8MB4 to support Chinese marketplace content.
- Input validation is applied to registration and management forms.
- Purchase operations keep product stock, sales totals and order records consistent.
- Empty states and confirmation dialogs provide safer, clearer user interactions.
- Product and order tables support practical filtering and management workflows.
- GitHub Actions performs a clean CMake build on every push and pull request.

## Future Development

- Replace direct SQL construction with prepared statements throughout the data layer
- Add automated unit and integration tests for purchase transactions
- Package the Windows application with an installer
- Add image upload storage and asynchronous loading
- Introduce role-based access control and password hashing suitable for production

## Author

Developed as a university software engineering and database project, with a focus on C++ desktop development, relational data design and user-centred interface implementation.
