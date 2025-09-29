# Ensure predictable behavior
setopt NO_BEEP
setopt INTERACTIVE_COMMENTS
setopt AUTO_CD

HISTFILE="$HOME/.zsh_history"
HISTSIZE=100000
SAVEHIST=100000

setopt APPEND_HISTORY
setopt SHARE_HISTORY
setopt HIST_IGNORE_DUPS
setopt HIST_IGNORE_ALL_DUPS
setopt HIST_REDUCE_BLANKS
setopt HIST_IGNORE_SPACE
setopt INC_APPEND_HISTORY

# Load and initialize zsh completion
autoload -Uz compinit

# Faster startup, safe on single-user systems
if [[ ! -f ~/.zcompdump ]]; then
  compinit
else
  compinit -C
fi

# Basic completion styling (optional but recommended)
zstyle ':completion:*' menu select
zstyle ':completion:*' matcher-list 'm:{a-z}={A-Za-z}'
zstyle ':completion:*' list-colors ${(s.:.)LS_COLORS}

# Arch package path
source /usr/share/zsh/plugins/zsh-autocomplete/zsh-autocomplete.plugin.zsh

# Optional: make autocomplete feel less aggressive
zstyle ':autocomplete:*' delay 0.05
zstyle ':autocomplete:*' min-input 1

source /usr/share/zsh/plugins/zsh-autosuggestions/zsh-autosuggestions.zsh

# Suggestion behavior
ZSH_AUTOSUGGEST_STRATEGY=(history completion)
ZSH_AUTOSUGGEST_HIGHLIGHT_STYLE='fg=8'

bindkey -e                       # Emacs keybindings
bindkey '^[[A' history-search-backward
bindkey '^[[B' history-search-forward

eval "$(starship init zsh)"
