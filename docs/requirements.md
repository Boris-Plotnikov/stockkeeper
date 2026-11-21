# Requirements

## Business goal

A small office (30–80 people) keeps furniture, IT equipment and supplies in one storage room. Today the stock is tracked in a spreadsheet. Problems: items run out without warning, nobody knows who took what, stocktaking differences cannot be explained.

## Users

- **Office manager** — receives deliveries, hands out items, orders new stock.
- **Finance** — needs the value of the warehouse at month end.

## User stories

1. **Receive goods.** As an office manager I want to record a delivery so that the stock is up to date.
   - Given item CH-100 has 14 pcs, when I run `receive CH-100 6`, then the stock is 20 and the journal has a RECEIVE line with balance 20.
2. **Hand out goods safely.** As an office manager I want the system to refuse issuing more than we have.
   - Given LP-310 has 7 pcs, when I run `issue LP-310 9`, then I see an error, the stock stays 7 and the journal is unchanged.
3. **Know what to reorder.** As an office manager I want a list of items below minimum stock with the quantity to order.
   - Given PA-010 has 2 boxes and minimum 10, when I run `low`, then I see "order 8 box".
4. **Stocktaking.** As an office manager I want to enter the counted quantity and keep the difference in the history.
5. **Month-end value.** As finance I want the total value of the warehouse.
6. **Do not lose work.** As a user I want to be warned when I quit with unsaved changes.

## Business rules

- Quantity is never negative.
- SKU is unique; names and SKUs cannot contain commas (CSV format).
- Prices are in EUR with at most 2 decimals.
- The journal is append-only.
