defmodule F1.Application do
  use Application

  # See https://hexdocs.pm/elixir/Application.html
  # for more information on OTP Applications
  @moduledoc false

  @target Mix.target()
  @version Mix.Project.config()[:version]

  alias Circuits.GPIO
  require Logger

  def start(_type, _args) do
    Logger.info("[ ==== F1 v#{@version} ==== ]")
    Logger.info("Starting pin #{@led_pin} as output")
    spawn(fn -> F1.start() end)
    children = []
    Supervisor.start_link(children, strategy: :one_for_one)
  end
end
