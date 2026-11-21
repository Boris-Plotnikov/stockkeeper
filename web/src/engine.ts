import create from "./wasm/stockkeeper_web.mjs";

export interface ItemRow {
  sku: string;
  name: string;
  unit: string;
  quantity: number;
  minStock: number;
  priceCents: number;
  low: boolean;
}

export interface MovementRow {
  id: number;
  date: string;
  sku: string;
  type: string;
  quantity: number;
  balanceAfter: number;
  note: string;
}

export interface Session {
  execute(line: string): string;
  reset(): void;
  finished(): boolean;
  unsaved(): boolean;
  items(): ItemRow[];
  journal(): MovementRow[];
  file(name: "items" | "movements"): string;
}

export async function startSession(): Promise<Session> {
  const module = (await create()) as { Session: new () => Session };
  return new module.Session();
}
