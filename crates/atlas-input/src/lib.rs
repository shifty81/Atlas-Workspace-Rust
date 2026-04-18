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

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn bind_and_query() {
        let mut mgr = InputManager::new();
        mgr.init();
        mgr.bind_action(InputAction::Jump, InputDevice::Keyboard, 32, "Jump");
        assert!(mgr.has_binding(InputAction::Jump));
        assert_eq!(mgr.binding_count(), 1);
        let b = mgr.get_binding(InputAction::Jump).unwrap();
        assert_eq!(b.key_code, 32);
        assert_eq!(b.name, "Jump");
    }

    #[test]
    fn press_held_release_cycle() {
        let mut mgr = InputManager::new();
        mgr.init();
        mgr.inject_press(InputAction::Jump);
        let s = mgr.get_state(InputAction::Jump);
        assert!(s.pressed);
        assert!(s.held);
        assert!(!s.released);
        assert!(mgr.is_pressed(InputAction::Jump));
        assert!(mgr.is_held(InputAction::Jump));

        // After update, pressed clears but held stays
        mgr.update();
        let s = mgr.get_state(InputAction::Jump);
        assert!(!s.pressed);
        assert!(s.held);

        // Release
        mgr.inject_release(InputAction::Jump);
        let s = mgr.get_state(InputAction::Jump);
        assert!(!s.held);
        assert!(s.released);
    }

    #[test]
    fn axis_value() {
        let mut mgr = InputManager::new();
        mgr.init();
        mgr.inject_axis(InputAction::MoveForward, 0.75);
        assert!((mgr.get_axis(InputAction::MoveForward) - 0.75).abs() < f32::EPSILON);
    }

    #[test]
    fn unbind_removes_binding() {
        let mut mgr = InputManager::new();
        mgr.bind_action(InputAction::Sprint, InputDevice::Keyboard, 160, "Sprint");
        assert!(mgr.has_binding(InputAction::Sprint));
        mgr.unbind_action(InputAction::Sprint);
        assert!(!mgr.has_binding(InputAction::Sprint));
        assert_eq!(mgr.binding_count(), 0);
    }

    #[test]
    fn unbound_state_is_default() {
        let mgr = InputManager::new();
        let s = mgr.get_state(InputAction::Interact);
        assert!(!s.pressed);
        assert!(!s.held);
        assert_eq!(s.value, 0.0);
    }

    #[test]
    fn update_clears_released() {
        let mut mgr = InputManager::new();
        mgr.inject_release(InputAction::Crouch);
        mgr.update();
        let s = mgr.get_state(InputAction::Crouch);
        assert!(!s.released);
    }

    #[test]
    fn multiple_actions_independent() {
        let mut mgr = InputManager::new();
        mgr.inject_press(InputAction::Jump);
        mgr.inject_press(InputAction::Sprint);
        assert!(mgr.is_pressed(InputAction::Jump));
        assert!(mgr.is_pressed(InputAction::Sprint));
        mgr.inject_release(InputAction::Jump);
        assert!(!mgr.is_held(InputAction::Jump));
        assert!(mgr.is_held(InputAction::Sprint));
    }
}
