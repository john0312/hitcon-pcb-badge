import yaml
import threading


class Config:
    """
    A singleton class to load and provide configuration from a YAML file.
    """

    _instance = None
    _lock = threading.Lock()

    def __new__(cls, config_file=None):
        with cls._lock:
            if cls._instance is None:
                cls._instance = super(Config, cls).__new__(cls)
                if config_file:
                    cls._instance._load_config(config_file)
                else:
                    cls._instance._config = {}  # Initialize with empty dict if no file.
        return cls._instance

    def _load_config(self, config_file):
        """Loads configuration from the specified YAML file."""
        try:
            with open(config_file, "r") as file:
                self._config = yaml.safe_load(file) or {}  # handle empty yaml file
        except FileNotFoundError:
            print(f"Error: Configuration file '{config_file}' not found.")
            self._config = {}
        except yaml.YAMLError as e:
            print(f"Error: Failed to parse YAML file '{config_file}': {e}")
            self._config = {}

    def get(self, key, default=None):
        """Retrieves a configuration value by key, with an optional default."""
        if self._config is None:
            return default
        return self._config.get(key, default)

    def __getitem__(self, key):
        """Allows accessing configuration values using dictionary-like syntax."""
        return self._config[key]

    def __contains__(self, key):
        """Allows checking if a key exists in the configuration."""
        return key in self._config

    def __iter__(self):
        """Allows iterating over the configuration keys."""
        return iter(self._config)

    def __len__(self):
        """Returns the number of configuration keys."""
        return len(self._config)
