defmodule F1Test do
  use ExUnit.Case
  doctest F1

  test "greets the world" do
    assert F1.hello() == :world
  end
end
