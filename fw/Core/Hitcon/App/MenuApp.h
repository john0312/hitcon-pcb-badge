#ifndef MENU_APP_BASE_H
#define MENU_APP_BASE_H

#include <App/app.h>

namespace hitcon {

/**
 * @brief Function pointer type for menu callbacks
 */
typedef void (*menu_callback_t)();

/**
 * @brief Represents a single menu entry
 */
struct menu_entry_t {
  const char *name;      ///< Display name of the menu item
  App *app;              ///< App to launch when selected (optional)
  menu_callback_t func;  ///< Callback function for menu item (optional)
};

/**
 * @brief Base class for menu-based applications
 *
 * Provides common functionality for menus including navigation
 * and item selection.
 */
class MenuApp : public App {
 public:
  MenuApp() = default;

  /**
   * @brief Construct a new Menu App object
   *
   * @param menu_entries Pointer to array of menu entries
   * @param menu_entry_size Number of entries in the array
   */
  MenuApp(const menu_entry_t *menu_entries, const int menu_entry_size);

  virtual ~MenuApp() = default;

  void OnEntry() override;
  void OnExit() override;
  void OnButton(button_t button) override;

  /**
   * @brief Called when MODE button is pressed
   */
  virtual void OnButtonMode() = 0;

  /**
   * @brief Called when BACK button is pressed
   */
  virtual void OnButtonBack() = 0;

  /**
   * @brief Called when BACK button is long-pressed
   */
  virtual void OnButtonLongBack() = 0;

  /**
   * @brief Change the menu size
   *
   * @param new_size New menu entry count
   * @param reset_index Reset selection index to 0 if true
   */
  void AdjustMenuSize(int new_size, bool reset_index = false);

  /**
   * @brief Change menu entries and reset selection
   *
   * @param new_menu New menu entries array
   * @param new_size New menu entry count
   * @param reset_index Reset selection index to 0 if true
   */
  void AdjustMenuPointer(const menu_entry_t *new_menu, int new_size,
                         bool reset_index = false);

 protected:
  const menu_entry_t *menu_entries;  ///< Array of menu entries
  int menu_entry_index;              ///< Currently selected menu index
  int menu_entry_size;               ///< Number of menu entries
  bool active;                       ///< Whether this app is active
};

}  // namespace hitcon

#endif  // MENU_APP_BASE_H
