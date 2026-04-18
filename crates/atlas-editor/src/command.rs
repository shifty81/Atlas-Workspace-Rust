//! [`EditorCommand`] trait and [`CommandStack`] for undo/redo (M7).

use atlas_ecs::World;

/// An atomic, reversible editor operation.
pub trait EditorCommand: Send + Sync {
    /// Human-readable description shown in the undo history.
    fn description(&self) -> &str;
    /// Apply the command to the world.
    fn apply(&mut self, world: &mut World);
    /// Revert the command (undo).
    fn revert(&mut self, world: &mut World);
}

/// Fixed-capacity undo / redo stack.
pub struct CommandStack {
    undo_stack: Vec<Box<dyn EditorCommand>>,
    redo_stack: Vec<Box<dyn EditorCommand>>,
    capacity:   usize,
}

impl CommandStack {
    pub fn new(capacity: usize) -> Self {
        Self { undo_stack: Vec::new(), redo_stack: Vec::new(), capacity }
    }

    /// Execute a command, push onto undo stack, and clear redo stack.
    pub fn execute(&mut self, mut cmd: Box<dyn EditorCommand>, world: &mut World) {
        cmd.apply(world);
        self.redo_stack.clear();
        self.undo_stack.push(cmd);
        if self.undo_stack.len() > self.capacity {
            self.undo_stack.remove(0);
        }
    }

    /// Undo the last command.
    pub fn undo(&mut self, world: &mut World) {
        if let Some(mut cmd) = self.undo_stack.pop() {
            cmd.revert(world);
            self.redo_stack.push(cmd);
        }
    }

    /// Redo the last undone command.
    pub fn redo(&mut self, world: &mut World) {
        if let Some(mut cmd) = self.redo_stack.pop() {
            cmd.apply(world);
            self.undo_stack.push(cmd);
        }
    }

    pub fn can_undo(&self) -> bool { !self.undo_stack.is_empty() }
    pub fn can_redo(&self) -> bool { !self.redo_stack.is_empty() }

    pub fn undo_description(&self) -> Option<&str> {
        self.undo_stack.last().map(|c| c.description())
    }
    pub fn redo_description(&self) -> Option<&str> {
        self.redo_stack.last().map(|c| c.description())
    }

    pub fn clear(&mut self) {
        self.undo_stack.clear();
        self.redo_stack.clear();
    }
}

impl Default for CommandStack {
    fn default() -> Self { Self::new(64) }
}
