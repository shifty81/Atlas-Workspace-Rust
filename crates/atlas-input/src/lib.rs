use std::collections::HashMap;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
#[repr(u32)]
pub enum InputAction {
    None = 0,
    MoveForward,
    MoveBackward,
    MoveLeft,
    MoveRight,
    Jump,
    Crouch,
    Sprint,
    Interact,
    PrimaryAction,
    SecondaryAction,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum InputDevice {
    Keyboard,
    Mouse,
    Gamepad,
}

#[derive(Debug, Clone)]
pub struct InputBinding {
    pub action: InputAction,
    pub device: InputDevice,
    pub key_code: u32,
    pub name: String,
}

#[derive(Debug, Clone, Default)]
pub struct InputState {
    pub pressed: bool,
    pub held: bool,
    pub released: bool,
    pub value: f32,
}

#[derive(Default)]
pub struct InputManager {
    bindings: HashMap<u32, InputBinding>,
    states: HashMap<u32, InputState>,
    initialized: bool,
}

impl InputManager {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn init(&mut self) {
        self.initialized = true;
        log::info!("InputManager initialized");
    }

    pub fn shutdown(&mut self) {
        self.initialized = false;
        self.bindings.clear();
        self.states.clear();
        log::info!("InputManager shutdown");
    }

    pub fn bind_action(&mut self, action: InputAction, device: InputDevice, key_code: u32, name: impl Into<String>) {
        let key = action as u32;
        self.bindings.insert(key, InputBinding { action, device, key_code, name: name.into() });
    }

    pub fn unbind_action(&mut self, action: InputAction) {
        self.bindings.remove(&(action as u32));
    }

    pub fn has_binding(&self, action: InputAction) -> bool {
        self.bindings.contains_key(&(action as u32))
    }

    pub fn get_binding(&self, action: InputAction) -> Option<&InputBinding> {
        self.bindings.get(&(action as u32))
    }

    pub fn binding_count(&self) -> usize {
        self.bindings.len()
    }

    pub fn inject_press(&mut self, action: InputAction) {
        let state = self.states.entry(action as u32).or_default();
        state.pressed = true;
        state.held = true;
        state.released = false;
    }

    pub fn inject_release(&mut self, action: InputAction) {
        let state = self.states.entry(action as u32).or_default();
        state.pressed = false;
        state.held = false;
        state.released = true;
    }

    pub fn inject_axis(&mut self, action: InputAction, value: f32) {
        let state = self.states.entry(action as u32).or_default();
        state.value = value;
    }

    pub fn get_state(&self, action: InputAction) -> InputState {
        self.states.get(&(action as u32)).cloned().unwrap_or_default()
    }

    pub fn is_pressed(&self, action: InputAction) -> bool {
        self.states.get(&(action as u32)).map_or(false, |s| s.pressed)
    }

    pub fn is_held(&self, action: InputAction) -> bool {
        self.states.get(&(action as u32)).map_or(false, |s| s.held)
    }

    pub fn get_axis(&self, action: InputAction) -> f32 {
        self.states.get(&(action as u32)).map_or(0.0, |s| s.value)
    }

    pub fn update(&mut self) {
        for state in self.states.values_mut() {
            if state.pressed {
                state.pressed = false;
                // held remains true
            }
            state.released = false;
        }
    }
}
