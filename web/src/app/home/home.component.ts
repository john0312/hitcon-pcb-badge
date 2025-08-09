import { AfterViewInit, Component, ElementRef, OnInit, ViewChild } from '@angular/core';
import { CommonModule } from '@angular/common';
import { StateService } from '../state.service';
import { AbsPipe } from '../abs.pipe';

@Component({
  selector: 'app-home',
  imports: [CommonModule, AbsPipe],
  templateUrl: './home.component.html',
  styleUrl: './home.component.css'
})
export class HomeComponent implements AfterViewInit, OnInit {
  @ViewChild('mapWrapper') mapWrapper?: ElementRef;

  readonly scalerMax = 299;
  readonly scalerMin = 79;
  floor = 0;
  scaler = 99;
  readonly floorImg: string[] = ['./3f.svg', './4f.svg'];

  // TODO: the data of points should be fetched from server periodically
  points0: point[] = [
    {
      x: 40, // 第二會議議右上
      y: 31.5,
      score: 0
    },
    {
      x: 43,  // 第二會議室最左
      y: 20.5,
      score: 1000
    },
    {
      x: 68,  // 第二會議議右下
      y: 29,
      score: -60
    },
    {
      x: 40, // 第一會議議左上
      y: 67.5,
      score: 50
    },
    {
      x: 58, // 第一會議議最右
      y: 80.5,
      score: -100
    },
    {
      x: 67, // 第一會議議左下
      y: 72.5,
      score: 120
    },
    {
      x: 16, // 遠距會議室左上
      y: 63.8,
      score: 70
    },
    {
      x: 27, // 遠距會議室右下
      y: 73,
      score: -66
    },
    {
      x: 84,  // 地圖中下
      y: 50,
      score: 100
    }
  ];

  points1: point[] = [
    {
      x: 90,  // 大會攤位左邊
      y: 32,
      score: 600
    },
    {
      x: 90,  // 大會攤位右邊
      y: 66,
      score: -450
    },
    {
      x: 73,  // 國際會議廳左下
      y: 36,
      score: 500
    },
    {
      x: 73,  // 國際會議廳右下
      y: 63,
      score: 300
    },
    {
      x: 49,  // 國際會議廳左上
      y: 40,
      score: -300
    },
    {
      x: 49,  // 國際會議廳右上
      y: 59,
      score: 300
    },
    {
      x: 65,  // 南棟下面
      y: 25,
      score: -800
    },
    {
      x: 35,  // 南棟上面
      y: 29.5,
      score: -170
    },
    {
      x: 65,  // 北棟下面
      y: 74.5,
      score: 180
    },
    {
      x: 35,  // 北棟上面
      y: 69.5,
      score: -700
    },
    {
      x: 17,  // 交誼廳
      y: 59,
      score: 700
    }
  ];

  points = [this.points0, this.points1];

  constructor(public state: StateService) { }

  ngAfterViewInit(): void {
    this.moveWrapper(this.state.scrollLeft, this.state.scrollTop);
  }

  ngOnInit() {
    this.floor = this.state.floor;
    this.scaler = this.state.scaler;
  }

  changeFloor(floor: number) {
    this.floor = floor;
    this.state.floor = this.floor;
  }

  zoom(delta: number) {
    this.scaler = Math.max(Math.min(this.scaler + delta, this.scalerMax), this.scalerMin);
    this.state.scaler = this.scaler;
  }

  moveWrapper(left: number | null, top: number | null) {
    if (!this.mapWrapper) {
      return;
    }
    const el = this.mapWrapper.nativeElement;
    if (left != null && top != null) {
      el.scrollLeft = Math.max(0, left);
      el.scrollTop = Math.max(0, top);
    } else {
      el.scrollLeft = Math.max(0, (el.scrollWidth - el.offsetWidth) / 2);
      el.scrollTop = Math.max(0, (el.scrollHeight - el.offsetHeight) / 2);
    }
  }

  onMapWrapperScroll(event: Event) {
    const target = event.target as HTMLElement;
    if (this.mapWrapper && this.mapWrapper.nativeElement === target) {
      this.state.scrollLeft = target.scrollLeft;
      this.state.scrollTop = target.scrollTop;
    }
  }

  // convert score to width on UI
  // lowest score: -1000
  // highest score: 1000
  // when the score is positive red > blue
  // when the score is negative blue > red
  // scoreToRingRadius represents the blue team's score
  scoreToRingRadius(score: number): number {
    return (1000 - score) / 2000 * 360;
  }
}

interface point {
  score: number // for each team score
  x: number // x-axis of position
  y: number // y-axis of position
}