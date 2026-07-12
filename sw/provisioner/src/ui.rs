use std::collections::VecDeque;
use std::sync::{Arc, Mutex};

use ratatui::Frame;
use ratatui::crossterm::event::{KeyCode, KeyEvent, KeyModifiers};
use ratatui::layout::{Constraint, Layout, Rect};
use ratatui::style::{Color, Style};
use ratatui::text::{Line, Span};
use ratatui::widgets::{Block, Borders, Cell, Paragraph, Row, Table};

use crate::firmware::{FirmwareSource, FwRegistry};
use crate::worker::{Level, Phase};

/// Max number of scrolling log lines kept (oldest dropped past this).
const MAX_LOGS: usize = 500;

/// Map a message level to its display color.
fn level_color(level: Level) -> Color {
    match level {
        Level::Info => Color::Gray,
        Level::Progress => Color::Cyan,
        Level::Success => Color::Green,
        Level::Warn => Color::Yellow,
        Level::Error => Color::Red,
    }
}

/// One line per worker (SN): serial, phase, level, and latest message.
struct WorkerRow {
    sn: String,
    phase: Phase,
    level: Level,
    message: String,
}

/// Which screen is shown.
enum Screen {
    Main,   // "what's flashing now" + workers + log
    Select, // full firmware list + add-by-path
}

/// TUI state: a table with one line per SN on top, a scrolling event log at the bottom.
/// Plus a firmware selector (shared with the workers) shown on the main screen, and a separate
/// select screen for the full per-year list and adding firmware by path.
pub(crate) struct Ui {
    /// Ordered by Connected time (earliest on top, latest at the end).
    rows: Vec<WorkerRow>,
    /// Event history: one line per phase change plus manager-side events.
    logs: VecDeque<String>,
    /// Firmware selection, shared with the workers (they read the active choice at flash time).
    registry: Arc<Mutex<FwRegistry>>,
    screen: Screen,
    /// Some(buffer) while typing a new firmware path on the select screen.
    add_input: Option<String>,
    /// Last failed add-path error, shown in red on the select screen; cleared on edit/cancel/success.
    add_error: Option<String>,
}

impl Ui {
    pub(crate) fn new(registry: Arc<Mutex<FwRegistry>>) -> Self {
        Ui {
            rows: Vec::new(),
            logs: VecDeque::new(),
            registry,
            screen: Screen::Main,
            add_input: None,
            add_error: None,
        }
    }

    /// Handle a key press; returns true if the app should quit.
    pub(crate) fn handle_key(&mut self, key: KeyEvent) -> bool {
        // Path-entry mode swallows everything except Enter/Esc/Backspace, so q/c are just text.
        if self.add_input.is_some() {
            match key.code {
                // Editing clears a previous error (the user is fixing the path).
                KeyCode::Char(c) => {
                    self.add_input.as_mut().unwrap().push(c);
                    self.add_error = None;
                }
                KeyCode::Backspace => {
                    self.add_input.as_mut().unwrap().pop();
                    self.add_error = None;
                }
                KeyCode::Esc => {
                    self.add_input = None;
                    self.add_error = None;
                }
                KeyCode::Enter => {
                    let path = self.add_input.as_deref().unwrap().trim().to_string();
                    if path.is_empty() {
                        self.add_input = None;
                        self.add_error = None;
                    } else {
                        let result = self.registry.lock().unwrap().add_to_active(&path);
                        match result {
                            Ok(()) => {
                                self.add_input = None;
                                self.add_error = None;
                                self.push_log(format!("已新增韌體: {path}"));
                            }
                            // Keep the typed path so the user can fix it; show the reason inline + in the log.
                            Err(e) => {
                                self.add_error = Some(format!("新增失敗: {e}"));
                                self.push_log(format!("新增韌體失敗（{path}）: {e}"));
                            }
                        }
                    }
                }
                _ => {}
            }
            return false;
        }

        // Ctrl-C quits from any screen.
        if key.code == KeyCode::Char('c') && key.modifiers.contains(KeyModifiers::CONTROL) {
            return true;
        }
        match self.screen {
            Screen::Main => match key.code {
                KeyCode::Char('q') => return true,
                KeyCode::Left => self.registry.lock().unwrap().prev_year(),
                KeyCode::Right => self.registry.lock().unwrap().next_year(),
                KeyCode::Char('f') => self.screen = Screen::Select,
                _ => {}
            },
            Screen::Select => match key.code {
                KeyCode::Left => self.registry.lock().unwrap().prev_year(),
                KeyCode::Right => self.registry.lock().unwrap().next_year(),
                KeyCode::Up => self.registry.lock().unwrap().select_prev(),
                KeyCode::Down => self.registry.lock().unwrap().select_next(),
                KeyCode::Char('a') => self.add_input = Some(String::new()),
                KeyCode::Enter | KeyCode::Esc => self.screen = Screen::Main,
                _ => {}
            },
        }
        false
    }

    /// Set an SN's line (phase + level + free-form message) and record it in the log. Creates the
    /// line the first time the SN appears (pushed to the end = latest connected, keeping order).
    pub(crate) fn set_status(
        &mut self,
        sn: &str,
        phase: Phase,
        level: Level,
        msg: impl Into<String>,
    ) {
        let message = msg.into();
        match self.rows.iter_mut().find(|r| r.sn == sn) {
            Some(row) => {
                row.phase = phase;
                row.level = level;
                row.message = message.clone();
            }
            None => self.rows.push(WorkerRow {
                sn: sn.to_string(),
                phase,
                level,
                message: message.clone(),
            }),
        }
        self.push_log(format!("[SN: {sn}] {phase}: {message}"));
    }

    /// Remove an SN's line from the display.
    pub(crate) fn remove(&mut self, sn: &str) {
        self.rows.retain(|r| r.sn != sn);
    }

    /// Append a log-only line for an event that should not touch the status column: manager-side
    /// commands (WakeUp/Kill) or a no-serial device.
    pub(crate) fn log(&mut self, msg: impl Into<String>) {
        self.push_log(msg.into());
    }

    fn push_log(&mut self, line: String) {
        self.logs.push_back(line);
        while self.logs.len() > MAX_LOGS {
            self.logs.pop_front();
        }
    }

    pub(crate) fn render(&self, frame: &mut Frame) {
        // One lock per frame; the workers only contend for it briefly at flash time.
        let reg = self.registry.lock().unwrap();
        match self.screen {
            Screen::Main => self.render_main(frame, &reg),
            Screen::Select => self.render_select(frame, &reg),
        }
    }

    /// Main screen: a one-line "what's flashing now" header, then workers and the log.
    fn render_main(&self, frame: &mut Frame, reg: &FwRegistry) {
        let [header, top, bottom] = Layout::vertical([
            Constraint::Length(3),
            Constraint::Min(3),
            Constraint::Percentage(40),
        ])
        .areas(frame.area());

        // Year tabs (active highlighted) + the active year's selected firmware.
        let mut spans = vec![Span::raw(" 硬體: ")];
        spans.extend(year_tabs(reg));
        spans.push(Span::raw(format!("   韌體: {}", reg.active_choice().label)));
        let header_widget = Paragraph::new(Line::from(spans)).block(
            Block::default()
                .borders(Borders::ALL)
                .title(" 現在要燒 (←/→ 切硬體年份 · f 選擇韌體 · q 離開) "),
        );
        frame.render_widget(header_widget, header);

        self.render_workers(frame, top);
        self.render_log(frame, bottom);
    }

    /// Select screen: year tabs, the active year's full firmware list, and the add-by-path row.
    fn render_select(&self, frame: &mut Frame, reg: &FwRegistry) {
        let mut lines: Vec<Line> = Vec::new();

        let mut tab_spans = vec![Span::raw(" 硬體:  ")];
        tab_spans.extend(year_tabs(reg));
        lines.push(Line::from(tab_spans));
        lines.push(Line::default());

        // The active year's choices; the selected one is marked and colored.
        let year = reg.active_year();
        lines.push(Line::from(format!(" {} 韌體:", year.year.label())));
        for (i, choice) in year.choices.iter().enumerate() {
            let selected = i == year.selected;
            let marker = if selected { " ▸ " } else { "   " };
            let color = if selected { Color::Cyan } else { Color::Gray };
            // File sources also show the full path (that's what identifies them).
            let text = match &choice.source {
                FirmwareSource::Embedded(_) => choice.label.clone(),
                FirmwareSource::File(p) => format!("{}  ({})", choice.label, p.display()),
            };
            lines.push(Line::styled(
                format!("{marker}{text}"),
                Style::default().fg(color),
            ));
        }
        lines.push(Line::default());

        // Add-by-path row: an input field while typing, a hint otherwise.
        match &self.add_input {
            Some(buf) => lines.push(Line::from(vec![
                Span::raw(" 新增路徑: "),
                Span::styled(format!("{buf}▌"), Style::default().fg(Color::Yellow)),
            ])),
            None => lines.push(Line::styled(
                " 新增路徑:  (按 a 開始輸入)",
                Style::default().fg(Color::DarkGray),
            )),
        }
        // Inline error right under the input, so a bad path is visible without the main-screen log.
        if let Some(err) = &self.add_error {
            lines.push(Line::styled(
                format!(" ⚠ {err}"),
                Style::default().fg(Color::Red),
            ));
        }
        lines.push(Line::default());

        let hint = if self.add_input.is_some() {
            " 輸入路徑…  Enter 確認 · Esc 取消"
        } else {
            " ←/→ 硬體年份   ↑/↓ 選韌體   a 新增路徑   Enter/Esc 返回"
        };
        lines.push(Line::styled(hint, Style::default().fg(Color::Cyan)));

        let widget =
            Paragraph::new(lines).block(Block::default().borders(Borders::ALL).title(" 選擇韌體 "));
        frame.render_widget(widget, frame.area());
    }

    fn render_workers(&self, frame: &mut Frame, area: Rect) {
        // Size the SN column to the longest serial currently shown (serial length varies by ST-Link
        // model), never below the "SN" header width.
        let sn_width = self
            .rows
            .iter()
            .map(|r| r.sn.chars().count())
            .max()
            .unwrap_or(0)
            .max(10) as u16;

        // Top: one line per worker; the phase and message cells are colored by the row's level.
        let rows = self.rows.iter().map(|r| {
            let style = Style::default().fg(level_color(r.level));
            Row::new([
                Cell::from(r.sn.clone()),
                Cell::from(Span::styled(r.phase.to_string(), style)),
                Cell::from(Span::styled(r.message.clone(), style)),
            ])
        });
        let table = Table::new(
            rows,
            [
                Constraint::Length(sn_width),
                Constraint::Length(13),
                Constraint::Min(0),
            ],
        )
        .header(Row::new(["SN", "階段", "訊息"]).style(Style::default().fg(Color::Cyan)))
        .block(
            Block::default()
                .borders(Borders::ALL)
                .title(format!(" Workers ({}) ", self.rows.len())),
        );
        frame.render_widget(table, area);
    }

    fn render_log(&self, frame: &mut Frame, area: Rect) {
        // Bottom: event log, take only the tail that fits (newest at the bottom).
        let inner_height = area.height.saturating_sub(2) as usize; // minus top/bottom borders
        let start = self.logs.len().saturating_sub(inner_height);
        let text: Vec<Line> = self
            .logs
            .iter()
            .skip(start)
            .map(|l| Line::from(l.as_str()))
            .collect();
        let log = Paragraph::new(text).block(Block::default().borders(Borders::ALL).title(" Log "));
        frame.render_widget(log, area);
    }
}

/// Year tabs for the header: the active year bracketed and highlighted, others dim.
fn year_tabs(reg: &FwRegistry) -> Vec<Span<'static>> {
    let mut spans = Vec::new();
    for (i, y) in reg.years.iter().enumerate() {
        let (text, style) = if i == reg.active {
            (
                format!("‹{}›", y.year.label()),
                Style::default().fg(Color::Black).bg(Color::Cyan),
            )
        } else {
            (
                format!(" {} ", y.year.label()),
                Style::default().fg(Color::Gray),
            )
        };
        spans.push(Span::styled(text, style));
        spans.push(Span::raw("  "));
    }
    spans
}
