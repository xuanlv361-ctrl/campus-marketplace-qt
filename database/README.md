# Database model

`schema.sql` creates an idempotent MySQL 8 schema for the marketplace and loads a compact, connected development dataset. It can be executed again after an update: catalogue rows use stable natural keys, and relationship seeds use unique keys or existence checks.

## Domain coverage

| Area | Tables |
| --- | --- |
| Identity and location | `User`, `Admin`, `Campus`, `UserAddress` |
| Catalogue | `Category`, `Goods`, `ProductAsset`, `GoodsImage` |
| Discovery and engagement | `Favorite`, `GoodsView`, `SearchHistory`, `Review` |
| Basket and checkout | `Cart`, `CartItem`, `Orders`, `Payment`, `Shipment` |
| Communication and trust | `Message`, `Notification`, `GoodsReport` |
| Operations | `AdminAuditLog` |

The schema also exposes four reporting views:

- `view_goods_detail`: listing, seller and category information.
- `view_order_detail`: buyer, seller, payment and fulfilment state.
- `view_category_sales`: listing, stock, sales and estimated revenue by category.
- `view_goods_engagement`: favourites, views, reviews and average rating by listing.

## Seed dataset

- 10 users and 1 administrator
- 11 categories
- 32 uniquely named marketplace listings
- 32 one-to-one primary product assets, with source and licence metadata
- Connected addresses, favourites, cart items, orders, payments, shipments, reviews and notifications

## Apply locally

From Command Prompt:

```bat
mysql -u root -p < database\schema.sql
```

Alternatively, open `schema.sql` in MySQL Workbench and execute the complete script. The C++ application reads connection settings from the `SECONDHAND_DB_HOST`, `SECONDHAND_DB_PORT`, `SECONDHAND_DB_NAME`, `SECONDHAND_DB_USER` and `SECONDHAND_DB_PASSWORD` environment variables.
