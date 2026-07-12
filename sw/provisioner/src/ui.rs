use std::collections::VecDeque;

use ratatui::Frame;
use ratatui::layout::{Constraint, Layout};
use ratatui::style::{Color, Style};
use ratatui::text::{Line, Span};
use ratatui::widgets::{Block, Borders, Cell, Paragraph, Row, Table};

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

/// TUI state: a table with one line per SN on top, a scrolling event log at the bottom.
pub(crate) struct Ui {
    /// Ordered by Connected time (earliest on top, latest at the end).
    rows: Vec<WorkerRow>,
    /// Event history: one line per phase change plus manager-side events.
    logs: VecDeque<String>,
}

impl Ui {
    pub(crate) fn new() -> Self {
        Ui {
            rows: Vec::new(),
            logs: VecDeque::new(),
        }
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
        let [top, bottom] =
            Layout::vertical([Constraint::Min(3), Constraint::Percentage(40)]).areas(frame.area());

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
        frame.render_widget(table, top);

        // Bottom: event log, take only the tail that fits (newest at the bottom).
        let inner_height = bottom.height.saturating_sub(2) as usize; // minus top/bottom borders
        let start = self.logs.len().saturating_sub(inner_height);
        let text: Vec<Line> = self
            .logs
            .iter()
            .skip(start)
            .map(|l| Line::from(l.as_str()))
            .collect();
        let log = Paragraph::new(text).block(
            Block::default()
                .borders(Borders::ALL)
                .title(" Log (q / Ctrl-C 離開) "),
        );
        frame.render_widget(log, bottom);
    }
}
