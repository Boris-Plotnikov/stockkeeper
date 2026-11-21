import { FormEvent, KeyboardEvent, useEffect, useMemo, useRef, useState } from "react";
import UnderTheHood, { Labels, useFlows } from "./codetour/UnderTheHood";
import { Flow, flowForRequest, logRequest, openTour } from "./codetour/store";
import { ItemRow, MovementRow, Session, startSession } from "./engine";

const LABELS: Labels = {
  subtitle: "Real C++ code of this program, taken from the repository at build time.",
  requests: "Live commands",
  emptyRequests: "Type a command in the terminal: each one appears here with the C++ code that handled it.",
};

const EXAMPLES = [
  "help",
  "low",
  'issue LP-310 2 for "Team Beta"',
  "issue PA-010 50",
  'receive PA-010 20 "Delivery 2026-118"',
  'count DS-200 2 "one desk damaged"',
  "history LP-310",
  "report",
  "save",
];

interface Line {
  id: number;
  input?: string;
  text: string;
  error?: boolean;
}

type Tab = "stock" | "journal" | "files";

const euro = (cents: number) =>
  (cents / 100).toLocaleString("en-US", { minimumFractionDigits: 2, maximumFractionDigits: 2 }) + " EUR";

function isError(output: string) {
  return output.startsWith("Error") || output.startsWith("Unknown command");
}

function Terminal({ session, onChange, flows }: { session: Session; onChange: () => void; flows: Flow[] | null }) {
  const [lines, setLines] = useState<Line[]>([]);
  const [input, setInput] = useState("");
  const [past, setPast] = useState<string[]>([]);
  const [cursor, setCursor] = useState(-1);
  const [ended, setEnded] = useState(false);
  const nextId = useRef(1);
  const screen = useRef<HTMLDivElement>(null);
  const field = useRef<HTMLInputElement>(null);

  useEffect(() => {
    setLines([{ id: 0, text: `StockKeeper - ${session.items().length} items loaded from data. Type 'help' for commands.` }]);
  }, [session]);

  useEffect(() => {
    screen.current?.scrollTo({ top: screen.current.scrollHeight });
  }, [lines]);

  const run = (command: string) => {
    const line = command.trim();
    if (!line || ended) return;
    const started = performance.now();
    const output = session.execute(line);
    const ms = Math.round((performance.now() - started) * 10) / 10;
    const error = isError(output);
    logRequest({ method: "RUN", path: line, status: error ? 400 : 200, ms, label: error ? "error" : "ok" });
    setLines((l) => [...l, { id: nextId.current++, input: line, text: output.replace(/\n$/, ""), error }]);
    setPast((p) => [line, ...p.filter((x) => x !== line)].slice(0, 30));
    setCursor(-1);
    setInput("");
    if (session.finished()) {
      setEnded(true);
      setLines((l) => [...l, { id: nextId.current++, text: "Program finished. Press “Reset” to start again." }]);
    }
    onChange();
    field.current?.focus({ preventScroll: true });
  };

  const submit = (e: FormEvent) => {
    e.preventDefault();
    run(input);
  };

  const onKey = (e: KeyboardEvent<HTMLInputElement>) => {
    if (e.key === "ArrowUp" && past.length) {
      e.preventDefault();
      const next = Math.min(cursor + 1, past.length - 1);
      setCursor(next);
      setInput(past[next]);
    } else if (e.key === "ArrowDown") {
      e.preventDefault();
      const next = cursor - 1;
      setCursor(next);
      setInput(next >= 0 ? past[next] : "");
    }
  };

  const reset = () => {
    session.reset();
    setEnded(false);
    setLines([{ id: nextId.current++, text: `Data reset. ${session.items().length} items loaded.` }]);
    onChange();
  };

  return (
    <div className="flex flex-col">
      <div className="overflow-hidden rounded-xl border border-slate-800 bg-[#0f172a] shadow-lg">
        <div className="flex items-center gap-2 border-b border-white/10 px-4 py-2">
          <span className="h-3 w-3 rounded-full bg-red-400/80" />
          <span className="h-3 w-3 rounded-full bg-amber-400/80" />
          <span className="h-3 w-3 rounded-full bg-emerald-400/80" />
          <span className="ml-2 font-mono text-xs text-slate-400">./stockkeeper data</span>
          <button onClick={reset} className="ml-auto text-xs text-slate-400 hover:text-white">Reset</button>
        </div>
        <div ref={screen} onClick={() => field.current?.focus()} className="h-[380px] overflow-y-auto px-4 py-3 font-mono text-[13px] leading-relaxed text-slate-200">
          {lines.map((l) => {
            const flow = l.input && flows ? flowForRequest(flows, "RUN", l.input) : undefined;
            return (
              <div key={l.id} className="mb-1">
                {l.input && (
                  <div className="flex items-center gap-2">
                    <span className="text-emerald-400">&gt;</span>
                    <span className="text-white">{l.input}</span>
                    {flow && (
                      <button onClick={() => openTour(flow.id)} className="ml-auto shrink-0 text-[11px] text-sky-300 hover:underline">
                        C++ code →
                      </button>
                    )}
                  </div>
                )}
                {l.text && <pre className={`whitespace-pre-wrap ${l.error ? "text-red-300" : "text-slate-300"}`}>{l.text}</pre>}
              </div>
            );
          })}
          <form onSubmit={submit} className="flex items-center gap-2">
            <span className="text-emerald-400">&gt;</span>
            <input
              ref={field}
              value={input}
              onChange={(e) => setInput(e.target.value)}
              onKeyDown={onKey}
              disabled={ended}
              autoFocus
              spellCheck={false}
              aria-label="Command"
              className="flex-1 bg-transparent text-white caret-emerald-400 outline-none"
            />
          </form>
        </div>
      </div>
      <div className="mt-3 flex flex-wrap gap-1.5">
        {EXAMPLES.map((e) => (
          <button key={e} onClick={() => run(e)} className="chip">{e}</button>
        ))}
      </div>
    </div>
  );
}

function StockTable({ items, changed }: { items: ItemRow[]; changed: Set<string> }) {
  return (
    <table className="w-full text-sm">
      <thead>
        <tr className="text-left text-xs uppercase tracking-wide text-muted">
          <th className="pb-2">SKU</th>
          <th className="pb-2">Item</th>
          <th className="pb-2 text-right">Stock</th>
          <th className="pb-2 text-right">Min</th>
          <th className="pb-2 text-right">Value</th>
        </tr>
      </thead>
      <tbody>
        {items.map((i) => (
          <tr key={i.sku} className={`border-t border-slate-100 transition-colors duration-700 ${changed.has(i.sku) ? "bg-brand/10" : ""}`}>
            <td className="py-1.5 font-mono text-xs">{i.sku}</td>
            <td className="py-1.5">{i.name}</td>
            <td className={`py-1.5 text-right font-semibold ${i.low ? "text-red-600" : ""}`}>{i.quantity} <span className="font-normal text-muted">{i.unit}</span></td>
            <td className="py-1.5 text-right text-muted">{i.minStock}</td>
            <td className="py-1.5 text-right">{euro(i.priceCents * i.quantity)}</td>
          </tr>
        ))}
      </tbody>
    </table>
  );
}

function Journal({ rows }: { rows: MovementRow[] }) {
  const last = rows.slice(-12).reverse();
  return (
    <div className="space-y-1.5">
      {last.map((m) => (
        <div key={m.id} className="flex items-baseline gap-2 text-sm">
          <span className="w-8 shrink-0 text-right font-mono text-xs text-muted">#{m.id}</span>
          <span className={`w-16 shrink-0 rounded px-1.5 text-center text-[10px] font-semibold ${m.type === "ISSUE" ? "bg-red-50 text-red-700" : m.type === "RECEIVE" ? "bg-emerald-50 text-emerald-700" : "bg-amber-50 text-amber-700"}`}>
            {m.type}
          </span>
          <span className="font-mono text-xs">{m.sku}</span>
          <span className="font-semibold">{m.type === "ISSUE" ? "−" : m.type === "RECEIVE" ? "+" : "="}{m.quantity}</span>
          <span className="text-xs text-muted">→ {m.balanceAfter}</span>
          <span className="min-w-0 flex-1 truncate text-xs text-slate-500" title={m.note}>{m.note}</span>
        </div>
      ))}
    </div>
  );
}

export default function App() {
  const [session, setSession] = useState<Session | null>(null);
  const [loadMs, setLoadMs] = useState(0);
  const [failed, setFailed] = useState(false);
  const [items, setItems] = useState<ItemRow[]>([]);
  const [journal, setJournal] = useState<MovementRow[]>([]);
  const [changed, setChanged] = useState<Set<string>>(new Set());
  const [tab, setTab] = useState<Tab>("stock");
  const [files, setFiles] = useState({ items: "", movements: "" });
  const flows = useFlows();

  useEffect(() => {
    const started = performance.now();
    startSession()
      .then((s) => {
        setLoadMs(Math.round(performance.now() - started));
        setSession(s);
        setItems(s.items());
        setJournal(s.journal());
        setFiles({ items: s.file("items"), movements: s.file("movements") });
      })
      .catch(() => setFailed(true));
  }, []);

  const refresh = () => {
    if (!session) return;
    const next = session.items();
    const before = new Map(items.map((i) => [i.sku, i.quantity]));
    setChanged(new Set(next.filter((i) => before.get(i.sku) !== i.quantity).map((i) => i.sku)));
    setTimeout(() => setChanged(new Set()), 1500);
    setItems(next);
    setJournal(session.journal());
    setFiles({ items: session.file("items"), movements: session.file("movements") });
  };

  const total = useMemo(() => items.reduce((s, i) => s + i.priceCents * i.quantity, 0), [items]);
  const low = items.filter((i) => i.low).length;

  return (
    <div className="min-h-screen">
      <header className="border-b border-slate-200 bg-white">
        <div className="mx-auto flex max-w-6xl items-center gap-3 px-4 py-3">
          <img src="./favicon.svg" alt="" className="h-7 w-7" />
          <div className="font-bold">StockKeeper</div>
          <span className="hidden text-sm text-muted sm:inline">C++17 · STL · CMake · GoogleTest</span>
          <a href="../" className="ml-auto text-sm text-muted hover:text-ink">← Portfolio</a>
        </div>
      </header>

      <main className="mx-auto max-w-6xl px-4 py-8">
        <h1 className="text-3xl font-bold tracking-tight">A command-line warehouse tool, running in your browser</h1>
        <p className="mt-2 max-w-3xl text-slate-600">
          This is the real C++ program — the same library as the terminal version and its 22 unit tests — compiled to WebAssembly with Emscripten.
          Nothing is sent to a server: every command below runs as C++ inside this page.
        </p>
        <div className="mt-3 flex flex-wrap gap-2 text-xs">
          <span className="rounded-full bg-emerald-100 px-2.5 py-1 font-semibold text-emerald-800">{session ? `● C++ module loaded in ${loadMs} ms` : failed ? "Could not load the module" : "Loading the C++ module…"}</span>
          <span className="rounded-full bg-slate-100 px-2.5 py-1 text-slate-700">Stock can never go negative</span>
          <span className="rounded-full bg-slate-100 px-2.5 py-1 text-slate-700">Money in cents, never double</span>
          <span className="rounded-full bg-slate-100 px-2.5 py-1 text-slate-700">Append-only journal</span>
        </div>

        {session && (
          <div className="mt-6 grid gap-6 lg:grid-cols-[1.1fr_1fr]">
            <Terminal session={session} onChange={refresh} flows={flows} />
            <div className="card flex flex-col p-4">
              <div className="mb-3 grid grid-cols-3 gap-2 text-center">
                <div><div className="text-xl font-bold">{items.length}</div><div className="text-xs text-muted">items</div></div>
                <div><div className={`text-xl font-bold ${low ? "text-red-600" : ""}`}>{low}</div><div className="text-xs text-muted">below minimum</div></div>
                <div><div className="text-xl font-bold">{euro(total)}</div><div className="text-xs text-muted">warehouse value</div></div>
              </div>
              <nav className="mb-3 flex gap-1 border-b border-slate-200">
                {([["stock", "Stock"], ["journal", `Journal (${journal.length})`], ["files", "CSV files"]] as [Tab, string][]).map(([id, label]) => (
                  <button key={id} onClick={() => setTab(id)}
                    className={`border-b-2 px-3 py-2 text-sm font-medium ${tab === id ? "border-brand text-brand-dark" : "border-transparent text-muted hover:text-ink"}`}>
                    {label}
                  </button>
                ))}
              </nav>
              <div className="max-h-[330px] flex-1 overflow-y-auto">
                {tab === "stock" && <StockTable items={items} changed={changed} />}
                {tab === "journal" && <Journal rows={journal} />}
                {tab === "files" && (
                  <div className="space-y-3">
                    <p className="text-xs text-muted">Files in the browser memory. Type <span className="font-mono">save</span> and watch them change.</p>
                    {(["items", "movements"] as const).map((f) => (
                      <div key={f}>
                        <div className="mb-1 font-mono text-xs font-semibold">{f}.csv</div>
                        <pre className="code-block max-h-40 overflow-auto rounded-lg p-3 text-[11px]">{files[f]}</pre>
                      </div>
                    ))}
                  </div>
                )}
              </div>
            </div>
          </div>
        )}

        <section className="mt-12">
          <h2 className="text-xl font-bold">How it is built</h2>
          <p className="mt-1 text-sm text-slate-600">Every command is a chain of real code: command parser → business logic → journal → unit test. Open any of them.</p>
          <div className="mt-4 grid gap-3 sm:grid-cols-2 lg:grid-cols-3">
            {(flows ?? []).map((f) => (
              <button key={f.id} onClick={() => openTour(f.id)} className="card p-4 text-left transition hover:border-brand/40">
                <div className="font-semibold">{f.title}</div>
                <p className="mt-1 text-xs leading-relaxed text-muted">{f.summary}</p>
                <div className="mt-2 font-mono text-xs text-brand">&lt;/&gt; {f.steps.length} snippets →</div>
              </button>
            ))}
          </div>
        </section>

        <section className="mt-12">
          <h2 className="text-xl font-bold">One C++ library, three ways to run it</h2>
          <div className="mt-4 grid gap-3 md:grid-cols-3">
            {[
              ["Terminal", "main.cpp reads lines from std::cin and prints what CommandLine::execute returns."],
              ["Unit tests", "GoogleTest calls the same execute() and checks the returned text — 22 tests in CI on Linux and Windows."],
              ["This page", "Emscripten compiles the library to WebAssembly; wasm/bindings.cpp exposes a small Session class to JavaScript."],
            ].map(([title, text]) => (
              <div key={title} className="card p-4">
                <div className="font-semibold">{title}</div>
                <p className="mt-1 text-sm text-slate-600">{text}</p>
              </div>
            ))}
          </div>
        </section>

        <footer className="mt-12 border-t border-slate-200 pt-6 text-sm text-muted">
          StockKeeper · a portfolio project by Boris Plotnikov · <a href="../" className="hover:text-ink">boris-plotnikov.click</a>
        </footer>
      </main>
      <UnderTheHood screen="/" labels={LABELS} />
    </div>
  );
}
