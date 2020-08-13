#include "app/services/config.hpp"
#include "app/services/controller.hpp"
#include "app/services/graphics.hpp"
#include "app/services/logic_thread.hpp"

#include "app/services/impl/runtime.hpp"

#include "lib/util/with_limits.hpp"

using namespace otto;
using namespace otto::services;

int main()
{
  auto app = start_app(ConfigManager::make_default(), LogicThread::make_default(), Controller::make_board(),
                       Graphics::make_board());

  std::cout << app.service<ConfigManager>().into_toml() << std::endl;

  struct Handler final : InputHandler {
    void handle(const KeyPress& e) noexcept override {}
    void handle(const KeyRelease& e) noexcept override {}
    void handle(const EncoderEvent& e) noexcept override
    {
      color = [&] {
        switch (e.encoder) {
          case Encoder::blue: return SK_ColorBLUE;
          case Encoder::green: return SK_ColorGREEN;
          case Encoder::yellow: return SK_ColorYELLOW;
          case Encoder::red: return SK_ColorRED;
        }
        OTTO_UNREACHABLE();
      }();
      n += e.steps;
    }
    SkColor color = SK_ColorWHITE;
    util::StaticallyBounded<int, 0, 160> n = 80;
  } handler;

  app.service<Graphics>().show([&](SkCanvas& ctx) {
    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    ctx.drawPaint(paint);
    paint.setColor(SK_ColorGREEN);
    paint.setStrokeJoin(SkPaint::kRound_Join);
    paint.setStrokeCap(SkPaint::kRound_Cap);
    ctx.drawRect({0, 0, 20, 20}, paint);
    paint.setColor(handler.color);
    ctx.drawRect({30 + handler.n, 30 + handler.n, 20, 20}, paint);
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kFill_Style);
    paint.setColor(handler.color);
    ctx.drawCircle({160, 120}, handler.n, paint);
    if (handler.n == 0) app.service<Runtime>().request_stop();
  });

  app.service<Controller>().set_input_handler(handler);
  app.wait_for_stop();
}
