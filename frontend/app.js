/* ==============================================
   Fraud Detection MPI — Dashboard Logic
   ==============================================
   Uses verified benchmark data from:
     results/native_fs_performance_table.md
   No benchmark values are invented or modified.
   ============================================== */

// ──────────────────────────────────────────────
// Verified Benchmark Data (source of truth)
// ──────────────────────────────────────────────
const BENCHMARK = {
  "10k": {
    rows: 10000,
    label: "paysim_10k.csv",
    rowsFormatted: "10,000",
    seq:  { io: 0.009703, comp: 0.000721, comm: 0.000000, total: 0.000721, flagged: 242 },
    np1:  { io: 0.011363, comp: 0.001471, comm: 0.000042, total: 0.001513, flagged: 242 },
    np2:  { io: 0.005185, comp: 0.000811, comm: 0.000058, total: 0.000869, flagged: 242 },
    np4:  { io: 0.025983, comp: 0.001292, comm: 0.018247, total: 0.019538, flagged: 242 }
  },
  "100k": {
    rows: 100000,
    label: "paysim_100k.csv",
    rowsFormatted: "100,000",
    seq:  { io: 0.101286, comp: 0.008798, comm: 0.000000, total: 0.008798, flagged: 3392 },
    np1:  { io: 0.104775, comp: 0.009996, comm: 0.000496, total: 0.010492, flagged: 3392 },
    np2:  { io: 0.080857, comp: 0.015992, comm: 0.000579, total: 0.016571, flagged: 3392 },
    np4:  { io: 0.060045, comp: 0.013316, comm: 0.001303, total: 0.014620, flagged: 3392 }
  },
  "1m": {
    rows: 1000000,
    label: "paysim_1m.csv",
    rowsFormatted: "1,000,000",
    seq:  { io: 1.063409, comp: 0.110947, comm: 0.000000, total: 0.110947, flagged: 31481 },
    np1:  { io: 0.974001, comp: 0.112388, comm: 0.005946, total: 0.118334, flagged: 31481 },
    np2:  { io: 0.685608, comp: 0.139042, comm: 0.009876, total: 0.148918, flagged: 31481 },
    np4:  { io: 0.485683, comp: 0.160981, comm: 0.010135, total: 0.171116, flagged: 31481 },
    np8:  { io: 0.618470, comp: 0.498817, comm: 0.044842, total: 0.543659, flagged: 31481 }
  },
  "full": {
    rows: 6362620,
    label: "paysim_full.csv",
    rowsFormatted: "6,362,620",
    seq:  { io: 7.241339, comp: 0.964967, comm: 0.000000, total: 0.964967, flagged: 81410 },
    np1:  { io: 7.382291, comp: 0.886887, comm: 0.033565, total: 0.920451, flagged: 81410 },
    np2:  { io: 3.603505, comp: 0.889127, comm: 0.028170, total: 0.917296, flagged: 81410 },
    np4:  { io: 3.177566, comp: 1.273878, comm: 0.057637, total: 1.331515, flagged: 81410 },
    np8:  { io: 4.880647, comp: 3.057386, comm: 0.238880, total: 3.296266, flagged: 81410 }
  }
};

const PROC_MAP = { seq: "seq", "1": "np1", "2": "np2", "4": "np4", "8": "np8" };
const PROC_LABELS = { seq: "Sequential", "1": "np = 1", "2": "np = 2", "4": "np = 4", "8": "np = 8" };

// ──────────────────────────────────────────────
// DOM Elements
// ──────────────────────────────────────────────
const datasetSelect = document.getElementById("datasetSelect");
const procsSelect = document.getElementById("procsSelect");
const btnRun = document.getElementById("btnRun");
const themeToggle = document.getElementById("themeToggle");
const themeIcon = document.getElementById("themeIcon");

const statusDataset = document.getElementById("statusDataset");
const statusProcs = document.getElementById("statusProcs");
const statusState = document.getElementById("statusState");

const metricTotal = document.getElementById("metricTotal");
const metricFlagged = document.getElementById("metricFlagged");
const metricIO = document.getElementById("metricIO");
const metricComp = document.getElementById("metricComp");
const metricComm = document.getElementById("metricComm");
const metricTotalTime = document.getElementById("metricTotalTime");

// ──────────────────────────────────────────────
// Theme
// ──────────────────────────────────────────────
function getTheme() {
  return localStorage.getItem("mpi-theme") || "light";
}

function applyTheme(theme) {
  document.documentElement.setAttribute("data-theme", theme);
  themeIcon.textContent = theme === "dark" ? "🌙" : "☀️";
  localStorage.setItem("mpi-theme", theme);
  updateChartsTheme();
}

themeToggle.addEventListener("click", () => {
  const next = getTheme() === "light" ? "dark" : "light";
  applyTheme(next);
});

// Initialize theme
applyTheme(getTheme());

// ──────────────────────────────────────────────
// Update Status Bar
// ──────────────────────────────────────────────
function updateStatus() {
  const ds = datasetSelect.value;
  const pr = procsSelect.value;
  statusDataset.textContent = BENCHMARK[ds].label;
  statusProcs.textContent = PROC_LABELS[pr];
}

datasetSelect.addEventListener("change", updateStatus);
procsSelect.addEventListener("change", updateStatus);
updateStatus();

// ──────────────────────────────────────────────
// Run Analysis (loads benchmark data)
// ──────────────────────────────────────────────
btnRun.addEventListener("click", () => {
  const ds = datasetSelect.value;
  const pr = procsSelect.value;
  const key = PROC_MAP[pr];
  const data = BENCHMARK[ds][key];
  const dataset = BENCHMARK[ds];

  // Status: loading benchmark data
  statusState.textContent = "Loading…";
  statusState.className = "status-value status-running";
  btnRun.classList.add("running");
  btnRun.disabled = true;

  // Clear metrics briefly while loading benchmark data
  metricTotal.textContent = "…";
  metricFlagged.textContent = "…";
  metricIO.textContent = "…";
  metricComp.textContent = "…";
  metricComm.textContent = "…";
  metricTotalTime.textContent = "…";

  setTimeout(() => {
    if (data) {
      // Populate metrics
      metricTotal.textContent = dataset.rowsFormatted;
      metricFlagged.textContent = data.flagged.toLocaleString();
      metricIO.textContent = data.io.toFixed(6);
      metricComp.textContent = data.comp.toFixed(6);
      metricComm.textContent = data.comm.toFixed(6);
      metricTotalTime.textContent = (data.io + data.total).toFixed(6);

      // Status: benchmark data loaded
      statusState.textContent = "Results Loaded";
      statusState.className = "status-value status-done";
    } else {
      metricTotal.textContent = dataset.rowsFormatted;
      metricFlagged.textContent = "N/A";
      metricIO.textContent = "—";
      metricComp.textContent = "—";
      metricComm.textContent = "—";
      metricTotalTime.textContent = "—";

      statusState.textContent = "Not Tested";
      statusState.className = "status-value";
    }
    btnRun.classList.remove("running");
    btnRun.disabled = false;

    // Update charts for selected dataset
    updateCharts(ds);
  }, 600);
});

// ──────────────────────────────────────────────
// Chart.js Configuration
// ──────────────────────────────────────────────
const chartInstances = {};

function getChartColors() {
  const isDark = getTheme() === "dark";
  return {
    grid: isDark ? "rgba(255,255,255,0.07)" : "rgba(0,0,0,0.06)",
    text: isDark ? "#9a9fb4" : "#5a6178",
    line1: "#4f6ef7",
    line2: "#10b981",
    line3: "#f59e0b",
    line4: "#ef4444",
    bg1: "rgba(79,110,247,0.1)",
    bg2: "rgba(16,185,129,0.1)",
    bg3: "rgba(245,158,11,0.1)",
    bg4: "rgba(239,68,68,0.1)"
  };
}

function baseChartOptions(colors) {
  return {
    responsive: true,
    maintainAspectRatio: false,
    animation: { duration: 500, easing: "easeOutCubic" },
    plugins: {
      legend: {
        display: true,
        position: "bottom",
        labels: {
          color: colors.text,
          font: { family: "'Inter', sans-serif", size: 11, weight: 500 },
          boxWidth: 10,
          padding: 14
        }
      },
      tooltip: {
        backgroundColor: getTheme() === "dark" ? "#22252f" : "#1a1e2e",
        titleFont: { family: "'Inter', sans-serif", size: 12, weight: 600 },
        bodyFont: { family: "'JetBrains Mono', monospace", size: 11 },
        padding: 10,
        cornerRadius: 6,
        displayColors: true,
        boxWidth: 8
      }
    },
    scales: {
      x: {
        grid: { color: colors.grid },
        ticks: { color: colors.text, font: { family: "'Inter', sans-serif", size: 11 } }
      },
      y: {
        grid: { color: colors.grid },
        ticks: { color: colors.text, font: { family: "'JetBrains Mono', monospace", size: 11 } }
      }
    }
  };
}

// ──────────────────────────────────────────────
// Chart 1: Total Execution Time vs MPI Processes
// ──────────────────────────────────────────────
function buildExecTimeChart(ds) {
  const colors = getChartColors();
  const d = BENCHMARK[ds];
  const labels = ["Sequential", "np=1", "np=2", "np=4"];
  const keys = ["seq", "np1", "np2", "np4"];
  // Total execution time = File I/O + Comp+Comm (as recorded in benchmarks)
  const totalExec = keys.map(k => d[k].io + d[k].total);
  const ioData = keys.map(k => d[k].io);
  const compCommData = keys.map(k => d[k].total);

  const opts = baseChartOptions(colors);
  opts.plugins.tooltip.callbacks = {
    afterBody: function(items) {
      const idx = items[0].dataIndex;
      return 'Total: ' + totalExec[idx].toFixed(4) + 's';
    }
  };
  opts.scales.y.title = { display: true, text: "Time (s)", color: colors.text, font: { family: "'Inter', sans-serif", size: 11 } };
  opts.scales.x.title = { display: true, text: "Configuration", color: colors.text, font: { family: "'Inter', sans-serif", size: 11 } };
  opts.scales.y.stacked = true;
  opts.scales.x.stacked = true;

  if (chartInstances.execTime) chartInstances.execTime.destroy();
  chartInstances.execTime = new Chart(document.getElementById("chartExecTime"), {
    type: "bar",
    data: {
      labels,
      datasets: [
        { label: "File I/O", data: ioData, backgroundColor: colors.line1, borderRadius: { topLeft: 0, topRight: 0, bottomLeft: 4, bottomRight: 4 }, barPercentage: 0.55 },
        { label: "Comp+Comm", data: compCommData, backgroundColor: colors.line2, borderRadius: { topLeft: 4, topRight: 4, bottomLeft: 0, bottomRight: 0 }, barPercentage: 0.55 }
      ]
    },
    options: opts
  });
}

// ──────────────────────────────────────────────
// Chart 2: Total-Time Speedup vs MPI Processes
// ──────────────────────────────────────────────
function buildSpeedupChart(ds) {
  const colors = getChartColors();
  const d = BENCHMARK[ds];
  const seqTotalTime = d.seq.io + d.seq.total;
  const labels = ["np=1", "np=2", "np=4"];
  const keys = ["np1", "np2", "np4"];
  // Total-time speedup: T_seq / T_mpi where T = I/O + Comp+Comm
  const totalSpeedup = keys.map(k => seqTotalTime / (d[k].io + d[k].total));
  // Ideal linear speedup for reference
  const idealSpeedup = [1, 2, 4].map(p => seqTotalTime > 0 ? p * (seqTotalTime / (d.seq.io + d.seq.total)) : p);
  // Simplified: ideal is just [1, 2, 4]
  const ideal = [1, 2, 4];

  const opts = baseChartOptions(colors);
  opts.scales.y.title = { display: true, text: "Speedup (×)", color: colors.text, font: { family: "'Inter', sans-serif", size: 11 } };
  opts.scales.y.min = 0;
  opts.scales.x.title = { display: true, text: "MPI Processes", color: colors.text, font: { family: "'Inter', sans-serif", size: 11 } };

  if (chartInstances.speedup) chartInstances.speedup.destroy();
  chartInstances.speedup = new Chart(document.getElementById("chartSpeedup"), {
    type: "line",
    data: {
      labels,
      datasets: [
        {
          label: "Actual Speedup",
          data: totalSpeedup,
          borderColor: colors.line1,
          backgroundColor: colors.bg1,
          tension: 0.3, fill: true, pointRadius: 5, pointHoverRadius: 7, borderWidth: 2
        },
        {
          label: "Ideal Linear",
          data: ideal,
          borderColor: colors.text,
          borderDash: [6, 4],
          backgroundColor: "transparent",
          tension: 0, fill: false, pointRadius: 3, pointHoverRadius: 5, borderWidth: 1.5
        }
      ]
    },
    options: opts
  });
}

// ──────────────────────────────────────────────
// Chart 3: Parallel Efficiency vs MPI Processes
// ──────────────────────────────────────────────
function buildEfficiencyChart(ds) {
  const colors = getChartColors();
  const d = BENCHMARK[ds];
  const seqTotal = d.seq.io + d.seq.total;
  const labels = ["np=1", "np=2", "np=4"];
  const procs = [1, 2, 4];

  const efficiency = procs.map((p, i) => {
    const keys = ["np1", "np2", "np4"];
    const mpiTotal = d[keys[i]].io + d[keys[i]].total;
    const speedup = seqTotal / mpiTotal;
    return (speedup / p) * 100;
  });

  const opts = baseChartOptions(colors);
  opts.scales.y.title = { display: true, text: "Efficiency (%)", color: colors.text, font: { family: "'Inter', sans-serif", size: 11 } };
  opts.scales.y.min = 0;
  opts.scales.y.max = Math.max(120, Math.ceil(Math.max(...efficiency) / 10) * 10 + 10);
  opts.scales.x.title = { display: true, text: "MPI Processes", color: colors.text, font: { family: "'Inter', sans-serif", size: 11 } };

  if (chartInstances.efficiency) chartInstances.efficiency.destroy();
  chartInstances.efficiency = new Chart(document.getElementById("chartEfficiency"), {
    type: "bar",
    data: {
      labels,
      datasets: [{
        label: "Parallel Efficiency",
        data: efficiency,
        backgroundColor: [colors.line2, colors.line3, colors.line4],
        borderRadius: 4,
        barPercentage: 0.5
      }]
    },
    options: opts
  });
}

// ──────────────────────────────────────────────
// Chart 4: Total Execution Time vs Dataset Size
// ──────────────────────────────────────────────
function buildDatasetSizeChart() {
  const colors = getChartColors();
  const labels = ["10K", "100K", "1M", "6.36M"];
  const keys = ["10k", "100k", "1m", "full"];

  // Total execution time = I/O + Comp+Comm for each configuration
  const seqTotals = keys.map(k => BENCHMARK[k].seq.io + BENCHMARK[k].seq.total);
  const np1Totals = keys.map(k => BENCHMARK[k].np1.io + BENCHMARK[k].np1.total);
  const np2Totals = keys.map(k => BENCHMARK[k].np2.io + BENCHMARK[k].np2.total);
  const np4Totals = keys.map(k => BENCHMARK[k].np4.io + BENCHMARK[k].np4.total);

  const opts = baseChartOptions(colors);
  opts.scales.y.title = { display: true, text: "Total Time (s)", color: colors.text, font: { family: "'Inter', sans-serif", size: 11 } };
  opts.scales.x.title = { display: true, text: "Dataset Size (rows)", color: colors.text, font: { family: "'Inter', sans-serif", size: 11 } };

  if (chartInstances.datasetSize) chartInstances.datasetSize.destroy();
  chartInstances.datasetSize = new Chart(document.getElementById("chartDatasetSize"), {
    type: "line",
    data: {
      labels,
      datasets: [
        {
          label: "Sequential",
          data: seqTotals,
          borderColor: colors.line4,
          backgroundColor: colors.bg4,
          tension: 0.3, fill: false, pointRadius: 5, pointHoverRadius: 7, borderWidth: 2
        },
        {
          label: "MPI np=1",
          data: np1Totals,
          borderColor: colors.line3,
          backgroundColor: colors.bg3,
          tension: 0.3, fill: false, pointRadius: 5, pointHoverRadius: 7, borderWidth: 2
        },
        {
          label: "MPI np=2",
          data: np2Totals,
          borderColor: colors.line1,
          backgroundColor: colors.bg1,
          tension: 0.3, fill: false, pointRadius: 5, pointHoverRadius: 7, borderWidth: 2
        },
        {
          label: "MPI np=4",
          data: np4Totals,
          borderColor: colors.line2,
          backgroundColor: colors.bg2,
          tension: 0.3, fill: false, pointRadius: 5, pointHoverRadius: 7, borderWidth: 2
        }
      ]
    },
    options: opts
  });
}

// ──────────────────────────────────────────────
// Chart Update Helpers
// ──────────────────────────────────────────────
function updateCharts(ds) {
  buildExecTimeChart(ds);
  buildSpeedupChart(ds);
  buildEfficiencyChart(ds);
  buildDatasetSizeChart();
}

function updateChartsTheme() {
  // Re-render all charts with the current dataset to pick up new colors
  const ds = datasetSelect.value;
  // Small delay to let CSS transition apply
  setTimeout(() => updateCharts(ds), 50);
}

// ──────────────────────────────────────────────
// Initialize — render charts with default dataset
// ──────────────────────────────────────────────
updateCharts("full");
