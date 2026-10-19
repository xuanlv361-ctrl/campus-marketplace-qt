# Campus Marketplace

A product-focused desktop marketplace for university communities, built with modern C++ and Qt Widgets. The application supports the complete journey from discovering and listing pre-owned items to purchasing, order tracking, moderation and operational reporting.

The application combines a consumer-facing marketplace experience with an operational administration dashboard. Its architecture is intended to remain useful beyond a classroom demo: shared data access, portable assets, bilingual interface support and traceable catalogue media are treated as product requirements.

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
- Runtime Chinese/English language switching with the selection persisted between windows and sessions
- Thirty-two seeded marketplace listings with a one-to-one product image mapping
- Extended relational schema for favourites, carts, messages, addresses, payments, fulfilment, reviews, notifications, moderation and licensed product assets

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

See [database/README.md](database/README.md) for the entity groups, reporting views and seed-data overview.

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

### Product catalogue assets

The seed catalogue contains 32 products across eleven categories. Every seeded product stores a dedicated relative `image_path`, and `ProductAsset` records the corresponding source URL and licence metadata. The schema also includes campus addresses, multi-image listings, payment and fulfilment records, reviews, notifications, reports, search history, view analytics and administrator audit logs. The bundled image files live in `ui_assets/products` and are copied beside the executable by CMake.

See [THIRD_PARTY_ASSETS.md](THIRD_PARTY_ASSETS.md) for the product-to-image mapping and source information.

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

Designed and developed as a portfolio-grade C++ desktop product, with a focus on marketplace workflows, relational data design and user-centred interface engineering.
